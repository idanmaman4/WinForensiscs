#pragma once

#include <Windows.h>
#define KDEXT_64BIT
#include <wdbgexts.h>
#include <DbgEng.h>

#include <cstdint>
#include <memory>

// Transparent cache in front of the dbgeng Ioctl routine obtained from
// IDebugControl3::GetWindbgExtensionApis64.  Windbg Efn extensions call this
// Ioctl to resolve types, sizes, fields and expressions repeatedly without
// caching.  We detour it once per process with polyhook2 and short-circuit
// the deterministic, symbol-keyed Ioctls using an in-memory cache.
//
// Caching is enabled for:
//   - IG_GET_TYPE_SIZE        (27)   - type_name   -> size
//   - IG_DUMP_SYMBOL_INFO     (22)   - SYM_DUMP_PARAM (name+options+fields) -> type id, size, field layout
//   - IG_GET_EXPRESSION_EX    (30)   - expression  -> value
//
// Any Ioctl that carries a caller callback (SYM_DUMP_PARAM.CallbackRoutine)
// or asks for memory-backed output (DBG_DUMP_COPY_TYPE_DATA, DBG_DUMP_READ_PHYSICAL)
// is passed through unchanged - those are not safely cacheable.
namespace IoctlHook
{
    // Resolve the real Ioctl routine and install the polyhook2 detour.
    // Returns true on success, false if the detour could not be placed or
    // if the hook is already installed.
    bool install();

    // Remove the detour (if installed) and drop the cached entries.
    void uninstall();

    // Drop all cached entries. Call after symbol reload / module reload
    // / process or thread switch so stale ids aren't served.
    void invalidate_cache();

    // True when the detour is currently active.
    bool is_installed();
}
