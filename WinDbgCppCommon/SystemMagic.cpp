#include "SystemMagic.h"

SystemMagic::SystemMagic(MasterDebugBridge& master_interface) : m_system(master_interface)
{

}

Expected<void> SystemMagic::attach_to_process(Address eprocess_address)
{
	HRESULT hr = m_system.get_interface()->SetImplicitProcessDataOffset(eprocess_address);
	if (FAILED(hr))
		return std::unexpected(std::exception("Failed to attach to process"));
	return {};
}

Expected<Address> SystemMagic::get_current_eprocess()
{
    Address eprocess;
    HRESULT hr = m_system.get_interface()->GetImplicitProcessDataOffset(&eprocess);
	if (FAILED(hr)) {
		throw std::exception("Failed to get eprocess");
	}

	return eprocess;
}

Expected<std::string> SystemMagic::get_current_process_name()
{
    char buf[MAX_PATH] = {};
    HRESULT hr = m_system.get_interface()->GetCurrentProcessExecutableName(
        buf, sizeof(buf), nullptr);

    if (FAILED(hr)) {
        return std::unexpected(std::exception("Can't get process name"));
    }

    return std::string(buf);
}


Expected<Address> SystemMagic::get_current_process_peb()
{
    ULONG64 peb = 0;
    HRESULT hr  = m_system.get_interface()->GetCurrentProcessPeb(&peb);

    if (FAILED(hr)) {
        return std::unexpected(std::exception("Can't get process name"));
    }

    return static_cast<Address>(peb);
}


Expected<void> SystemMagic::attach_to_thread(ULONG thread_id)
{
    HRESULT hr = m_system.get_interface()->SetCurrentThreadId(thread_id);

    if (FAILED(hr)) {
        return std::unexpected(std::exception("Can't get process name"));
    }

    return {};
}


Expected<Address> SystemMagic::get_current_thread_teb()
{
    Address teb = 0;
    HRESULT hr  = m_system.get_interface()->GetCurrentThreadTeb(&teb);

    if (FAILED(hr)) {
        return std::unexpected(std::exception("Can't get process name"));
    }

    return teb;
}
