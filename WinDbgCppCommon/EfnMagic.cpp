#include "EfnMagic.h"

#include <cstring>

using std::unexpected;
using std::exception;


EfnMagic::EfnMagic(MasterDebugBridge& master_bridge)
    : m_master(master_bridge),
      m_client(master_bridge),
      m_control(master_bridge)
{
}


EfnMagic::~EfnMagic()
{
    for (auto& kv : m_ext_handles) {
        m_control.get_interface()->RemoveExtension(kv.second);
    }
}


ULONG64 EfnMagic::load_extension(const char* ext_dll)
{
    auto it = m_ext_handles.find(ext_dll);
    if (it != m_ext_handles.end()) {
        return it->second;
    }

    ULONG64 handle = 0;
    HRESULT hr = m_control.get_interface()->AddExtension(ext_dll, 0, &handle);
    if (FAILED(hr) || handle == 0) {
        // Extension chain search fallback: zero handle tells GetExtensionFunction to walk.
        // Record a zero handle so future calls still try walk-mode instead of re-AddExtension.
        m_ext_handles[ext_dll] = 0;
        return 0;
    }

    m_ext_handles[ext_dll] = handle;
    return handle;
}


// =============================================================================
// kdexts.dll
// =============================================================================

Expected<DEBUG_DEVICE_OBJECT_INFO> EfnMagic::get_device_object_info(Address device_object)
{
    auto fn = resolve<PGET_DEVICE_OBJECT_INFO>(EfnExt::KDEXTS, "GetDevObjInfo");
    if (!fn) return unexpected(exception("Cannot resolve _EFN_GetDevObjInfo"));

    DEBUG_DEVICE_OBJECT_INFO info{};
    info.SizeOfStruct = sizeof(info);
    HRESULT hr = fn(m_client.get_interface(), device_object, &info);
    if (FAILED(hr)) return unexpected(exception("_EFN_GetDevObjInfo failed"));
    return info;
}


Expected<DEBUG_DRIVER_OBJECT_INFO> EfnMagic::get_driver_object_info(Address driver_object)
{
    auto fn = resolve<PGET_DRIVER_OBJECT_INFO>(EfnExt::KDEXTS, "GetDrvObjInfo");
    if (!fn) return unexpected(exception("Cannot resolve _EFN_GetDrvObjInfo"));

    DEBUG_DRIVER_OBJECT_INFO info{};
    info.SizeOfStruct = sizeof(info);
    HRESULT hr = fn(m_client.get_interface(), driver_object, &info);
    if (FAILED(hr)) return unexpected(exception("_EFN_GetDrvObjInfo failed"));
    return info;
}


Expected<std::vector<PROCESS_COMMIT_USAGE>> EfnMagic::get_process_commit(ULONG64& total_commit_charge_out)
{
    auto fn = resolve<PGET_PROCESS_COMMIT>(EfnExt::KDEXTS, "GetProcessCommit");
    if (!fn) return unexpected(exception("Cannot resolve _EFN_GetProcessCommit"));

    ULONG64 total  = 0;
    ULONG   count  = 0;
    PPROCESS_COMMIT_USAGE data = nullptr;
    HRESULT hr = fn(m_client.get_interface(), &total, &count, &data);
    if (FAILED(hr) || data == nullptr) {
        return unexpected(exception("_EFN_GetProcessCommit failed"));
    }

    total_commit_charge_out = total;
    std::vector<PROCESS_COMMIT_USAGE> out(data, data + count);
    return out;
}


Expected<std::string> EfnMagic::get_full_image_name(Address process)
{
    auto fn = resolve<PGET_FULL_IMAGE_NAME>(EfnExt::KDEXTS, "GetFullImageName");
    if (!fn) return unexpected(exception("Cannot resolve _EFN_GetFullImageName"));

    LPSTR  name = nullptr;
    HRESULT hr  = fn(m_client.get_interface(), process, &name);
    if (FAILED(hr) || name == nullptr) {
        return unexpected(exception("_EFN_GetFullImageName failed"));
    }
    return std::string(name);
}


Expected<DEBUG_CPU_SPEED_INFO> EfnMagic::get_cpu_speed_info()
{
    auto fn = resolve<PGET_CPU_PSPEED_INFO>(EfnExt::KDEXTS, "GetCpuSpeedInfo");
    if (!fn) return unexpected(exception("Cannot resolve _EFN_GetCpuSpeedInfo"));

    DEBUG_CPU_SPEED_INFO info{};
    info.SizeOfStruct = sizeof(info);
    HRESULT hr = fn(m_client.get_interface(), &info);
    if (FAILED(hr)) return unexpected(exception("_EFN_GetCpuSpeedInfo failed"));
    return info;
}


Expected<DEBUG_CPU_MICROCODE_VERSION> EfnMagic::get_cpu_microcode_version()
{
    auto fn = resolve<PGET_CPU_MICROCODE_VERSION>(EfnExt::KDEXTS, "GetCpuMicrocodeVersion");
    if (!fn) return unexpected(exception("Cannot resolve _EFN_GetCpuMicrocodeVersion"));

    DEBUG_CPU_MICROCODE_VERSION info{};
    info.SizeOfStruct = sizeof(info);
    HRESULT hr = fn(m_client.get_interface(), &info);
    if (FAILED(hr)) return unexpected(exception("_EFN_GetCpuMicrocodeVersion failed"));
    return info;
}


Expected<DEBUG_SMBIOS_INFO> EfnMagic::get_smbios_info()
{
    auto fn = resolve<PGET_SMBIOS_INFO>(EfnExt::KDEXTS, "GetSmbiosInfo");
    if (!fn) return unexpected(exception("Cannot resolve _EFN_GetSmbiosInfo"));

    DEBUG_SMBIOS_INFO info{};
    info.SizeOfStruct = sizeof(info);
    HRESULT hr = fn(m_client.get_interface(), &info);
    if (FAILED(hr)) return unexpected(exception("_EFN_GetSmbiosInfo failed"));
    return info;
}


Expected<DEBUG_IRP_INFO> EfnMagic::get_irp_info(Address irp)
{
    auto fn = resolve<PGET_IRP_INFO>(EfnExt::KDEXTS, "GetIrpInfo");
    if (!fn) return unexpected(exception("Cannot resolve _EFN_GetIrpInfo"));

    DEBUG_IRP_INFO info{};
    info.SizeOfStruct = sizeof(info);
    HRESULT hr = fn(m_client.get_interface(), irp, &info);
    if (FAILED(hr)) return unexpected(exception("_EFN_GetIrpInfo failed"));
    return info;
}


Expected<DEBUG_PNP_TRIAGE_INFO> EfnMagic::get_pnp_triage_info()
{
    auto fn = resolve<PGET_PNP_TRIAGE_INFO>(EfnExt::KDEXTS, "GetPNPTriageInfo");
    if (!fn) return unexpected(exception("Cannot resolve _EFN_GetPNPTriageInfo"));

    DEBUG_PNP_TRIAGE_INFO info{};
    info.SizeOfStruct = sizeof(info);
    HRESULT hr = fn(m_client.get_interface(), &info);
    if (FAILED(hr)) return unexpected(exception("_EFN_GetPNPTriageInfo failed"));
    return info;
}


Expected<DEBUG_POOL_DATA> EfnMagic::get_pool_data(Address pool)
{
    auto fn = resolve<PGET_POOL_DATA>(EfnExt::KDEXTS, "GetPoolData");
    if (!fn) return unexpected(exception("Cannot resolve _EFN_GetPoolData"));

    DEBUG_POOL_DATA data{};
    data.SizeofStruct = sizeof(data);
    HRESULT hr = fn(m_client.get_interface(), pool, &data);
    if (FAILED(hr)) return unexpected(exception("_EFN_GetPoolData failed"));
    return data;
}


Expected<DEBUG_POOL_REGION> EfnMagic::get_pool_region(Address pool)
{
    auto fn = resolve<PGET_POOL_REGION>(EfnExt::KDEXTS, "GetPoolRegion");
    if (!fn) return unexpected(exception("Cannot resolve _EFN_GetPoolRegion"));

    DEBUG_POOL_REGION region = DbgPoolRegionUnknown;
    HRESULT hr = fn(m_client.get_interface(), pool, &region);
    if (FAILED(hr)) return unexpected(exception("_EFN_GetPoolRegion failed"));
    return region;
}


Expected<KDEXT_THREAD_FIND_PARAMS> EfnMagic::find_matching_thread(const KDEXT_THREAD_FIND_PARAMS& query)
{
    auto fn = resolve<PFIND_MATCHING_THREAD>(EfnExt::KDEXTS, "FindMatchingThread");
    if (!fn) return unexpected(exception("Cannot resolve _EFN_FindMatchingThread"));

    KDEXT_THREAD_FIND_PARAMS params = query;
    params.SizeofStruct = sizeof(params);
    HRESULT hr = fn(m_client.get_interface(), &params);
    if (FAILED(hr)) return unexpected(exception("_EFN_FindMatchingThread failed"));
    return params;
}


Expected<Address> EfnMagic::find_matching_process(const KDEXT_PROCESS_FIND_PARAMS& query)
{
    auto fn = resolve<PFIND_MATCHING_PROCESS>(EfnExt::KDEXTS, "FindMatchingProcess");
    if (!fn) return unexpected(exception("Cannot resolve _EFN_FindMatchingProcess"));

    KDEXT_PROCESS_FIND_PARAMS params = query;
    params.SizeofStruct = sizeof(params);
    ULONG64 process = 0;
    HRESULT hr = fn(m_client.get_interface(), &params, &process);
    if (FAILED(hr)) return unexpected(exception("_EFN_FindMatchingProcess failed"));
    return static_cast<Address>(process);
}


HRESULT EfnMagic::enumerate_job_processes(Address job, EXTS_JOB_PROCESS_CALLBACK cb, void* ctx)
{
    auto fn = resolve<PENUMERATE_JOB_PROCESSES>(EfnExt::KDEXTS, "EnumerateJobProcesses");
    if (!fn) return E_NOINTERFACE;
    return fn(m_client.get_interface(), job, cb, ctx);
}


HRESULT EfnMagic::enumerate_hash_table(Address hash_table, EXTS_TABLE_ENTRY_CALLBACK cb, void* ctx)
{
    auto fn = resolve<PENUMERATE_HASH_TABLE>(EfnExt::KDEXTS, "EnumerateHashTable");
    if (!fn) return E_NOINTERFACE;
    return fn(m_client.get_interface(), hash_table, cb, ctx);
}


HRESULT EfnMagic::enumerate_handles(Address process, Address handle_to_dump, ULONG flags,
                                    KDEXT_DUMP_HANDLE_CALLBACK cb, void* ctx)
{
    auto fn = resolve<PENUMERATE_HANDLES>(EfnExt::KDEXTS, "EnumerateHandles");
    if (!fn) return E_NOINTERFACE;
    return fn(m_client.get_interface(), process, handle_to_dump, flags, cb, ctx);
}


Expected<KDEXT_FILELOCK_OWNER> EfnMagic::find_filelock_owner_info(Address file_object)
{
    auto fn = resolve<PFIND_FILELOCK_OWNERINFO>(EfnExt::KDEXTS, "FindFileLockOwnerInfo");
    if (!fn) return unexpected(exception("Cannot resolve _EFN_FindFileLockOwnerInfo"));

    KDEXT_FILELOCK_OWNER owner{};
    owner.Sizeofstruct = sizeof(owner);
    owner.FileObject   = file_object;
    HRESULT hr = fn(m_client.get_interface(), &owner);
    if (FAILED(hr)) return unexpected(exception("_EFN_FindFileLockOwnerInfo failed"));
    return owner;
}


HRESULT EfnMagic::enumerate_system_locks(ULONG flags, KDEXTS_LOCK_CALLBACKROUTINE cb, void* ctx)
{
    auto fn = resolve<PENUMERATE_SYSTEM_LOCKS>(EfnExt::KDEXTS, "EnumerateSystemLocks");
    if (!fn) return E_NOINTERFACE;
    return fn(m_client.get_interface(), flags, cb, ctx);
}


Expected<KDEXTS_PTE_INFO> EfnMagic::get_pte_info(Address virtual_address)
{
    auto fn = resolve<PKDEXTS_GET_PTE_INFO>(EfnExt::KDEXTS, "GetPteInfo");
    if (!fn) return unexpected(exception("Cannot resolve _EFN_GetPteInfo"));

    KDEXTS_PTE_INFO pte{};
    pte.SizeOfStruct   = sizeof(pte);
    pte.VirtualAddress = virtual_address;
    HRESULT hr = fn(m_client.get_interface(), virtual_address, &pte);
    if (FAILED(hr)) return unexpected(exception("_EFN_GetPteInfo failed"));
    return pte;
}


// =============================================================================
// kext.dll
// =============================================================================

Expected<DEBUG_POOLTAG_DESCRIPTION> EfnMagic::get_pool_tag_description(ULONG pool_tag)
{
    auto fn = resolve<PGET_POOL_TAG_DESCRIPTION>(EfnExt::KEXT, "GetPoolTagDescription");
    if (!fn) return unexpected(exception("Cannot resolve _EFN_GetPoolTagDescription"));

    DEBUG_POOLTAG_DESCRIPTION desc{};
    desc.SizeOfStruct = sizeof(desc);
    HRESULT hr = fn(pool_tag, &desc);
    if (FAILED(hr)) return unexpected(exception("_EFN_GetPoolTagDescription failed"));
    return desc;
}


// =============================================================================
// ext.dll  (!analyze family)
// =============================================================================

Expected<IDebugFailureAnalysis*> EfnMagic::get_failure_analysis(ULONG flags)
{
    auto fn = resolve<EXT_GET_FAILURE_ANALYSIS>(EfnExt::EXT, "GetFailureAnalysis");
    if (!fn) return unexpected(exception("Cannot resolve _EFN_GetFailureAnalysis"));

    IDebugClient4* c4 = nullptr;
    HRESULT hr = m_client.get_interface()->QueryInterface(__uuidof(IDebugClient4), (void**)&c4);
    if (FAILED(hr) || c4 == nullptr) return unexpected(exception("IDebugClient4 unavailable"));

    IDebugFailureAnalysis* analysis = nullptr;
    hr = fn(c4, flags, &analysis);
    c4->Release();
    if (FAILED(hr)) return unexpected(exception("_EFN_GetFailureAnalysis failed"));
    return analysis;
}


Expected<IDebugFailureAnalysis2*> EfnMagic::get_debug_failure_analysis(ULONG flags, REFCLSID clsid)
{
    auto fn = resolve<EXT_GET_DEBUG_FAILURE_ANALYSIS>(EfnExt::EXT, "GetDebugFailureAnalysis");
    if (!fn) return unexpected(exception("Cannot resolve _EFN_GetDebugFailureAnalysis"));

    IDebugClient4* c4 = nullptr;
    HRESULT hr = m_client.get_interface()->QueryInterface(__uuidof(IDebugClient4), (void**)&c4);
    if (FAILED(hr) || c4 == nullptr) return unexpected(exception("IDebugClient4 unavailable"));

    IDebugFailureAnalysis2* analysis = nullptr;
    hr = fn(c4, flags, clsid, &analysis);
    c4->Release();
    if (FAILED(hr)) return unexpected(exception("_EFN_GetDebugFailureAnalysis failed"));
    return analysis;
}


Expected<void*> EfnMagic::create_failure_analysis_instance(PCWSTR args, ULONG flags, REFCLSID clsid, REFIID riid)
{
    auto fn = resolve<fnDebugFailureAnalysisCreateInstance>(EfnExt::EXT, "DebugFailureAnalysisCreateInstance");
    if (!fn) return unexpected(exception("Cannot resolve _EFN_DebugFailureAnalysisCreateInstance"));

    LPVOID out = nullptr;
    HRESULT hr = fn(m_client.get_interface(), args, flags, clsid, riid, &out);
    if (FAILED(hr) || out == nullptr) return unexpected(exception("_EFN_DebugFailureAnalysisCreateInstance failed"));
    return out;
}


HRESULT EfnMagic::run_analysis_plugin(FA_EXTENSION_PLUGIN_PHASE phase, PDEBUG_FAILURE_ANALYSIS2 analysis, const std::string& plugin_name)
{
    auto fn = resolve<EXT_ANALYSIS_PLUGIN>(EfnExt::EXT, plugin_name.c_str());
    if (!fn) return E_NOINTERFACE;

    IDebugClient4* c4 = nullptr;
    HRESULT hr = m_client.get_interface()->QueryInterface(__uuidof(IDebugClient4), (void**)&c4);
    if (FAILED(hr) || c4 == nullptr) return hr;

    hr = fn(c4, phase, analysis);
    c4->Release();
    return hr;
}


Expected<EfnMagic::FaEntriesData> EfnMagic::get_failure_analysis_entries_data()
{
    auto fn = resolve<EXT_GET_FA_ENTRIES_DATA>(EfnExt::EXT, "GetFaEntriesData");
    if (!fn) return unexpected(exception("Cannot resolve _EFN_GetFaEntriesData"));

    IDebugClient4* c4 = nullptr;
    HRESULT hr = m_client.get_interface()->QueryInterface(__uuidof(IDebugClient4), (void**)&c4);
    if (FAILED(hr) || c4 == nullptr) return unexpected(exception("IDebugClient4 unavailable"));

    ULONG     count   = 0;
    PFA_ENTRY entries = nullptr;
    hr = fn(c4, &count, &entries);
    c4->Release();
    if (FAILED(hr) || entries == nullptr) return unexpected(exception("_EFN_GetFaEntriesData failed"));

    return FaEntriesData{ entries, count };
}


Expected<TARGET_DEBUG_INFO> EfnMagic::get_target_info()
{
    auto fn = resolve<EXT_TARGET_INFO>(EfnExt::EXT, "GetTargetInfo");
    if (!fn) return unexpected(exception("Cannot resolve _EFN_GetTargetInfo"));

    IDebugClient4* c4 = nullptr;
    HRESULT hr = m_client.get_interface()->QueryInterface(__uuidof(IDebugClient4), (void**)&c4);
    if (FAILED(hr) || c4 == nullptr) return unexpected(exception("IDebugClient4 unavailable"));

    TARGET_DEBUG_INFO info{};
    info.SizeOfStruct = sizeof(info);
    hr = fn(c4, &info);
    c4->Release();
    if (FAILED(hr)) return unexpected(exception("_EFN_GetTargetInfo failed"));
    return info;
}


void EfnMagic::decode_error(DEBUG_DECODE_ERROR& decode)
{
    auto fn = resolve<EXT_DECODE_ERROR>(EfnExt::EXT, "DecodeError");
    if (!fn) return;
    decode.SizeOfStruct = sizeof(decode);
    fn(&decode);
}


DWORD EfnMagic::get_triage_followup_from_symbol(PCSTR symbol_name, DEBUG_TRIAGE_FOLLOWUP_INFO& owner_info)
{
    auto fn = resolve<EXT_TRIAGE_FOLLOWUP>(EfnExt::EXT, "GetTriageFollowupFromSymbol");
    if (!fn) return TRIAGE_FOLLOWUP_FAIL;

    IDebugClient4* c4 = nullptr;
    HRESULT hr = m_client.get_interface()->QueryInterface(__uuidof(IDebugClient4), (void**)&c4);
    if (FAILED(hr) || c4 == nullptr) return TRIAGE_FOLLOWUP_FAIL;

    owner_info.SizeOfStruct = sizeof(owner_info);
    DWORD result = fn(c4, symbol_name, &owner_info);
    c4->Release();
    return result;
}


HRESULT EfnMagic::reload_triager()
{
    auto fn = resolve<EXT_RELOAD_TRIAGER>(EfnExt::EXT, "ReloadTriager");
    if (!fn) return E_NOINTERFACE;

    IDebugClient4* c4 = nullptr;
    HRESULT hr = m_client.get_interface()->QueryInterface(__uuidof(IDebugClient4), (void**)&c4);
    if (FAILED(hr) || c4 == nullptr) return hr;

    hr = fn(c4);
    c4->Release();
    return hr;
}


HRESULT EfnMagic::ext_xml_data(PEXT_CAB_XML_DATA xml)
{
    auto fn = resolve<EXT_XML_DATA>(EfnExt::EXT, "XmlData");
    if (!fn) return E_NOINTERFACE;

    IDebugClient4* c4 = nullptr;
    HRESULT hr = m_client.get_interface()->QueryInterface(__uuidof(IDebugClient4), (void**)&c4);
    if (FAILED(hr) || c4 == nullptr) return hr;

    if (xml) xml->SizeOfStruct = sizeof(*xml);
    hr = fn(c4, xml);
    c4->Release();
    return hr;
}


HRESULT EfnMagic::query_data_by_tag(ULONG tag, void* query_info, PBYTE data, ULONG data_size)
{
    auto fn = resolve<EXTDLL_QUERYDATABYTAG>(EfnExt::EXT, "ExtDllQueryDataByTag");
    if (!fn) return E_NOINTERFACE;

    IDebugClient4* c4 = nullptr;
    HRESULT hr = m_client.get_interface()->QueryInterface(__uuidof(IDebugClient4), (void**)&c4);
    if (FAILED(hr) || c4 == nullptr) return hr;

    hr = fn(c4, tag, query_info, data, data_size);
    c4->Release();
    return hr;
}


HRESULT EfnMagic::query_data_by_tag_ex(ULONG tag, void* query_info, PBYTE data, ULONG data_size, PBYTE data_ex, ULONG data_ex_size)
{
    auto fn = resolve<EXTDLL_QUERYDATABYTAGEX>(EfnExt::EXT, "ExtDllQueryDataByTagEx");
    if (!fn) return E_NOINTERFACE;

    IDebugClient4* c4 = nullptr;
    HRESULT hr = m_client.get_interface()->QueryInterface(__uuidof(IDebugClient4), (void**)&c4);
    if (FAILED(hr) || c4 == nullptr) return hr;

    hr = fn(c4, tag, query_info, data, data_size, data_ex, data_ex_size);
    c4->Release();
    return hr;
}


void EfnMagic::iterate_rtl_balanced_nodes(Address root, ULONG entry_offset, ENTRY_CALLBACK cb, void* ctx)
{
    auto fn = resolve<EXTDLL_ITERATERTLBALANCEDNODES>(EfnExt::EXT, "IterateRtlBalancedNodes");
    if (!fn) return;
    fn(root, entry_offset, cb, ctx);
}


// =============================================================================
// ntsdexts.dll
// =============================================================================

HRESULT EfnMagic::get_handle_trace(ULONG trace_type, ULONG start_index,
                                   ULONG64* handle_value, ULONG64* stack_functions, ULONG stack_trace_size)
{
    auto fn = resolve<EXT_GET_HANDLE_TRACE>(EfnExt::NTSDEXTS, "GetHandleTrace");
    if (!fn) return E_NOINTERFACE;
    return fn(m_client.get_interface(), trace_type, start_index, handle_value, stack_functions, stack_trace_size);
}


// =============================================================================
// exts.dll
// =============================================================================

Expected<std::string> EfnMagic::get_environment_variable(Address peb, const std::string& variable)
{
    auto fn = resolve<EXT_GET_ENVIRONMENT_VARIABLE>(EfnExt::EXTS, "GetEnvironmentVariable");
    if (!fn) return unexpected(exception("Cannot resolve _EFN_GetEnvironmentVariable"));

    std::string buffer(MAX_PATH * 4, '\0');
    HRESULT hr = fn(peb, const_cast<PSTR>(variable.c_str()), buffer.data(), static_cast<ULONG>(buffer.size()));
    if (FAILED(hr)) return unexpected(exception("_EFN_GetEnvironmentVariable failed"));

    buffer.resize(std::strlen(buffer.data()));
    return buffer;
}
