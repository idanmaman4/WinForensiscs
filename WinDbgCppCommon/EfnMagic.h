#pragma once

#include <Windows.h>
#define KDEXT_64BIT
#include <wdbgexts.h>
#include <DbgEng.h>
#include "extsfns.h"

#include <string>
#include <unordered_map>
#include <vector>

#include "Types.h"
#include "DebugBridge.h"

// Known extension dlls that export _EFN_ functions.
namespace EfnExt
{
    constexpr const char* KDEXTS    = "kdexts";
    constexpr const char* KEXT      = "kext";
    constexpr const char* EXT       = "ext";
    constexpr const char* NTSDEXTS  = "ntsdexts";
    constexpr const char* EXTS      = "exts";
}

class EfnMagic
{
public:
    explicit EfnMagic(MasterDebugBridge& master_bridge);
    ~EfnMagic();

    EfnMagic(const EfnMagic&)            = delete;
    EfnMagic& operator=(const EfnMagic&) = delete;

    // --- kdexts.dll -----------------------------------------------------------
    Expected<DEBUG_DEVICE_OBJECT_INFO>     get_device_object_info(Address device_object);
    Expected<DEBUG_DRIVER_OBJECT_INFO>     get_driver_object_info(Address driver_object);
    Expected<std::vector<PROCESS_COMMIT_USAGE>> get_process_commit(ULONG64& total_commit_charge_out);
    Expected<std::string>                  get_full_image_name(Address process);
    Expected<DEBUG_CPU_SPEED_INFO>         get_cpu_speed_info();
    Expected<DEBUG_CPU_MICROCODE_VERSION>  get_cpu_microcode_version();
    Expected<DEBUG_SMBIOS_INFO>            get_smbios_info();
    Expected<DEBUG_IRP_INFO>               get_irp_info(Address irp);
    Expected<DEBUG_PNP_TRIAGE_INFO>        get_pnp_triage_info();
    Expected<DEBUG_POOL_DATA>              get_pool_data(Address pool);
    Expected<DEBUG_POOL_REGION>            get_pool_region(Address pool);
    Expected<KDEXT_THREAD_FIND_PARAMS>     find_matching_thread(const KDEXT_THREAD_FIND_PARAMS& query);
    Expected<Address>                      find_matching_process(const KDEXT_PROCESS_FIND_PARAMS& query);
    HRESULT                                enumerate_job_processes(Address job, EXTS_JOB_PROCESS_CALLBACK cb, void* ctx);
    HRESULT                                enumerate_hash_table(Address hash_table, EXTS_TABLE_ENTRY_CALLBACK cb, void* ctx);
    HRESULT                                enumerate_handles(Address process, Address handle_to_dump, ULONG flags,
                                                             KDEXT_DUMP_HANDLE_CALLBACK cb, void* ctx);
    Expected<KDEXT_FILELOCK_OWNER>         find_filelock_owner_info(Address file_object);
    HRESULT                                enumerate_system_locks(ULONG flags, KDEXTS_LOCK_CALLBACKROUTINE cb, void* ctx);
    Expected<KDEXTS_PTE_INFO>              get_pte_info(Address virtual_address);

    // --- kext.dll -------------------------------------------------------------
    Expected<DEBUG_POOLTAG_DESCRIPTION>    get_pool_tag_description(ULONG pool_tag);

    // --- ext.dll (analyze family) --------------------------------------------
    Expected<IDebugFailureAnalysis*>       get_failure_analysis(ULONG flags);
    Expected<IDebugFailureAnalysis2*>      get_debug_failure_analysis(ULONG flags, REFCLSID clsid);
    Expected<void*>                        create_failure_analysis_instance(PCWSTR args, ULONG flags, REFCLSID clsid, REFIID riid);
    HRESULT                                run_analysis_plugin(FA_EXTENSION_PLUGIN_PHASE phase, PDEBUG_FAILURE_ANALYSIS2 analysis, const std::string& plugin_name);
    // FA_ENTRY is a variable-length header (see FA_ENTRY.FullSize) so the
    // caller is expected to walk the returned pointer using FullSize stride.
    struct FaEntriesData { PFA_ENTRY first; ULONG count; };
    Expected<FaEntriesData>                get_failure_analysis_entries_data();
    Expected<TARGET_DEBUG_INFO>            get_target_info();
    void                                   decode_error(DEBUG_DECODE_ERROR& decode);
    DWORD                                  get_triage_followup_from_symbol(PCSTR symbol_name, DEBUG_TRIAGE_FOLLOWUP_INFO& owner_info);
    HRESULT                                reload_triager();
    HRESULT                                ext_xml_data(PEXT_CAB_XML_DATA xml);
    HRESULT                                query_data_by_tag(ULONG tag, void* query_info, PBYTE data, ULONG data_size);
    HRESULT                                query_data_by_tag_ex(ULONG tag, void* query_info, PBYTE data, ULONG data_size, PBYTE data_ex, ULONG data_ex_size);
    void                                   iterate_rtl_balanced_nodes(Address root, ULONG entry_offset, ENTRY_CALLBACK cb, void* ctx);

    // --- ntsdexts.dll ---------------------------------------------------------
    HRESULT                                get_handle_trace(ULONG trace_type, ULONG start_index,
                                                            ULONG64* handle_value, ULONG64* stack_functions, ULONG stack_trace_size);

    // --- exts.dll -------------------------------------------------------------
    Expected<std::string>                  get_environment_variable(Address peb, const std::string& variable);

private:
    template <typename FN>
    FN resolve(const char* ext_dll, const char* fn_name);

    ULONG64 load_extension(const char* ext_dll);

    MasterDebugBridge&                          m_master;
    DebugBridge<IDebugClient>                   m_client;
    DebugBridge<IDebugControl3>                 m_control;

    std::unordered_map<std::string, ULONG64>    m_ext_handles;   // ext_dll -> handle
    std::unordered_map<std::string, FARPROC>    m_fn_cache;      // "ext_dll!fn_name" -> fn ptr
};


template <typename FN>
inline FN EfnMagic::resolve(const char* ext_dll, const char* fn_name)
{
    std::string key = std::string(ext_dll) + "!" + fn_name;
    auto it = m_fn_cache.find(key);
    if (it != m_fn_cache.end()) {
        return reinterpret_cast<FN>(it->second);
    }

    ULONG64 handle = load_extension(ext_dll);
    if (handle == 0) {
        return nullptr;
    }

    FARPROC raw = nullptr;
    HRESULT hr  = m_control.get_interface()->GetExtensionFunction(handle, fn_name, &raw);
    if (FAILED(hr) || raw == nullptr) {
        return nullptr;
    }

    m_fn_cache[key] = raw;
    return reinterpret_cast<FN>(raw);
}
