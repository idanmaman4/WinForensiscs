#include "CommandMagic.h"
#include "DebugMagic.h"

#include <dbgeng.h>
#include <sstream>
#include <string>

CommandMagic::CommandMagic()
{
}

// ---------------------------------------------------------------------------
// Minimal IDebugOutputCallbacks that accumulates all text into a std::string.
// ---------------------------------------------------------------------------
namespace {

class OutputCapture final : public IDebugOutputCallbacks
{
public:
    // IUnknown – no real ref-counting needed; lifetime is stack-scoped.
    ULONG   STDMETHODCALLTYPE AddRef()  override { return 1; }
    ULONG   STDMETHODCALLTYPE Release() override { return 1; }
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, void**) override { return E_NOINTERFACE; }

    // IDebugOutputCallbacks
    HRESULT STDMETHODCALLTYPE Output(ULONG /*Mask*/, PCSTR Text) override
    {
        if (Text) m_buf += Text;
        return S_OK;
    }

    const std::string& str() const { return m_buf; }
    void               clear()     { m_buf.clear(); }

private:
    std::string m_buf;
};


// ---------------------------------------------------------------------------
// Parse the text produced by !vad into a vector of VadEntry.
//
// Expected line format (whitespace-separated columns):
//   <vad_addr>  <level>  <start_vpn>  <end_vpn>  <commit>
//   <type>  [Exe]  <protection>  [file-name...]
// ---------------------------------------------------------------------------
std::vector<VadEntry> parse_vad_output(const std::string& raw)
{
    std::vector<VadEntry> entries;
    std::istringstream stream(raw);
    std::string line;

    while (std::getline(stream, line)) {
        if (line.empty()) continue;

        // Skip the header line ("VAD  Level  Start  End  Commit")
        if (line.find("VAD") != std::string::npos &&
            line.find("Level") != std::string::npos)
            continue;

        std::istringstream ls(line);
        std::string vad_s, lvl_s, start_s, end_s, commit_s, type_s, next_s;

        if (!(ls >> vad_s >> lvl_s >> start_s >> end_s >> commit_s >> type_s >> next_s))
            continue;

        VadEntry e{};

        try { e.vad_address   = std::stoull(vad_s,   nullptr, 16); } catch (...) { continue; }
        try { e.level         = (ULONG)std::stoul(lvl_s,   nullptr, 10); } catch (...) { continue; }
        try { e.starting_vpn  = std::stoull(start_s, nullptr, 16); } catch (...) { continue; }
        try { e.ending_vpn    = std::stoull(end_s,   nullptr, 16); } catch (...) { continue; }
        try { e.commit_charge = std::stoull(commit_s, nullptr, 10); } catch (...) {}

        e.private_memory = (type_s == "Private");
        e.vad_type       = e.private_memory ? 0 : 1;

        // next_s is either "Exe" (image-mapped) or the protection string
        std::string prot_s;
        if (next_s == "Exe") {
            e.vad_type = 2;
            if (!(ls >> prot_s)) prot_s.clear();
        } else {
            prot_s = next_s;
        }

        // Map protection string to numeric code
        if      (prot_s == "READONLY")           e.protection = 1;
        else if (prot_s == "EXECUTE")             e.protection = 2;
        else if (prot_s == "EXECUTE_READ")        e.protection = 3;
        else if (prot_s == "READWRITE")           e.protection = 4;
        else if (prot_s == "WRITECOPY")           e.protection = 5;
        else if (prot_s == "EXECUTE_READWRITE")   e.protection = 6;
        else if (prot_s == "EXECUTE_WRITECOPY")   e.protection = 7;
        else if (prot_s == "NO_ACCESS")           e.protection = 8;

        // Everything remaining on the line is either a file path or a section description.
        // A Windows file path always begins with '\'; everything else is metadata text
        // (e.g. "Pagefile section, shared commit 0x6" or "Can't read file name buffer at ...").
        std::string rest;
        if (std::getline(ls, rest)) {
            size_t first = rest.find_first_not_of(" \t");
            if (first != std::string::npos) {
                rest = rest.substr(first);
                while (!rest.empty() && (rest.back() == ' ' || rest.back() == '\r'))
                    rest.pop_back();
                if (!rest.empty()) {
                    if (rest.front() == '\\')
                        e.file_name = std::move(rest);
                    else
                        e.section_info = std::move(rest);
                }
            }
        }

        entries.push_back(std::move(e));
    }

    return entries;
}

} // anonymous namespace


// ---------------------------------------------------------------------------
// Public method: execute !vad in the target process context and cache results.
// ---------------------------------------------------------------------------
Expected<std::vector<VadEntry>> CommandMagic::parse_vad(DebugMagic& dm,
                                                         Address eprocess_address)
{
    auto it = m_vad_cache.find(eprocess_address);
    if (it != m_vad_cache.end())
        return it->second;

    // Save the current implicit process so we can restore it afterwards.
    auto saved_ep = dm.system().get_current_eprocess();

    // Switch implicit process to the target EPROCESS so !vad reads its VAD tree.
    dm.system().attach_to_process(eprocess_address);

    // ---- capture !vad output -----------------------------------------------
    IDebugClient* client = dm.master().get_interface();

    IDebugOutputCallbacks* old_cb = nullptr;
    client->GetOutputCallbacks(&old_cb);

    OutputCapture capture;
    client->SetOutputCallbacks(&capture);

    IDebugControl* ctrl = nullptr;
    HRESULT hr = client->QueryInterface(__uuidof(IDebugControl), (void**)&ctrl);
    if (SUCCEEDED(hr)) {
        ctrl->Execute(DEBUG_OUTCTL_THIS_CLIENT, "!vad", DEBUG_EXECUTE_DEFAULT);
        ctrl->Release();
    }

    client->SetOutputCallbacks(old_cb);
    if (old_cb) old_cb->Release();
    // -------------------------------------------------------------------------

    // Restore the process context that was active before this call.
    if (saved_ep.has_value())
        dm.system().attach_to_process(saved_ep.value());

    if (FAILED(hr))
        return std::unexpected(std::exception("CommandMagic: failed to acquire IDebugControl"));

    auto entries = parse_vad_output(capture.str());
    m_vad_cache[eprocess_address] = entries;
    return entries;
}
