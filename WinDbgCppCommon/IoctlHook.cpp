#include "IoctlHook.h"

#include <polyhook2/Detour/x64Detour.hpp>
#include <polyhook2/ZydisDisassembler.hpp>

#include <atomic>
#include <cstring>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#pragma comment(lib, "dbgeng.lib")

namespace
{
    // ---------------------------------------------------------------------
    // polyhook2 detour state.  Kept in an anonymous namespace so it's only
    // visible to the hook implementation.
    // ---------------------------------------------------------------------
    std::atomic<bool>                    g_installed{false};
    std::unique_ptr<PLH::x64Detour>      g_detour;
    uint64_t                             g_trampoline = 0;

    // Always route pass-through calls through the trampoline address that
    // polyhook2 populates, never through the original function address -
    // the original is now patched and would bounce straight back into our
    // hook.
    inline PWINDBG_IOCTL_ROUTINE original_ioctl()
    {
        return reinterpret_cast<PWINDBG_IOCTL_ROUTINE>(g_trampoline);
    }

    // ---------------------------------------------------------------------
    // Cache storage.
    // The cache is split per Ioctl type; each entry is keyed on the caller's
    // input parameters and stores the scalar outputs we need to restore.
    // Single-thread assumption: no mutex is needed because the hooked Ioctl
    // routine is only driven from the primary dbgeng client thread.
    // ---------------------------------------------------------------------

    // IG_GET_TYPE_SIZE
    struct TypeSizeValue {
        ULONG return_code;
        // IG_GET_TYPE_SIZE returns size directly as the Ioctl return code;
        // there is no additional out parameter we need to restore.
    };
    std::unordered_map<std::string, TypeSizeValue> g_type_size_cache;

    // IG_GET_EXPRESSION_EX
    struct ExpressionValue {
        ULONG   return_code;
        ULONG64 value;
        ULONG64 remainder;
    };
    std::unordered_map<std::string, ExpressionValue> g_expression_cache;

    // IG_DUMP_SYMBOL_INFO
    struct FieldOutput {
        ULONG   size;
        ULONG64 address;
        ULONG   type_id;
        ULONG   field_offset;
        ULONG   buffer_size;
        USHORT  bit_pos;
        USHORT  bit_size;
        ULONG   f_pointer;
        ULONG   f_array;
        ULONG   f_struct;
        ULONG   f_constant;
        ULONG   f_static;
    };

    struct DumpSymbolValue {
        ULONG                    return_code;
        ULONG64                  mod_base;
        ULONG                    type_id;
        ULONG                    type_size;
        ULONG                    f_pointer;
        ULONG                    f_array;
        ULONG                    f_struct;
        ULONG                    f_constant;
        std::vector<FieldOutput> fields;   // one entry per SYM_DUMP_PARAM.Fields[i]
    };
    std::unordered_map<std::string, DumpSymbolValue> g_dump_symbol_cache;

    // ---------------------------------------------------------------------
    // Key builders
    // ---------------------------------------------------------------------
    std::string build_type_size_key(PSYM_DUMP_PARAM sym)
    {
        std::string key;
        key.reserve(64);
        if (sym->sName) key.append(reinterpret_cast<const char*>(sym->sName));
        key.push_back('|');
        char buf[64];
        std::snprintf(buf, sizeof(buf), "%llx|%x", (unsigned long long)sym->ModBase, sym->TypeId);
        key.append(buf);
        return key;
    }

    std::string build_dump_symbol_key(PSYM_DUMP_PARAM sym)
    {
        std::string key;
        key.reserve(128);
        if (sym->sName) key.append(reinterpret_cast<const char*>(sym->sName));
        key.push_back('|');
        char buf[96];
        std::snprintf(buf, sizeof(buf), "o=%x|mb=%llx|tid=%x|nf=%u",
                      sym->Options,
                      (unsigned long long)sym->ModBase,
                      sym->TypeId,
                      sym->nFields);
        key.append(buf);
        if (sym->nFields && sym->Fields) {
            for (ULONG i = 0; i < sym->nFields; ++i) {
                key.push_back('|');
                if (sym->Fields[i].fName) {
                    key.append(reinterpret_cast<const char*>(sym->Fields[i].fName));
                }
                std::snprintf(buf, sizeof(buf), ":fo=%x", sym->Fields[i].fOptions);
                key.append(buf);
            }
        }
        return key;
    }

    // ---------------------------------------------------------------------
    // Safety predicates
    // ---------------------------------------------------------------------
    bool is_dump_symbol_cacheable(PSYM_DUMP_PARAM sym)
    {
        if (!sym || sym->size < sizeof(SYM_DUMP_PARAM)) return false;
        if (sym->CallbackRoutine != nullptr)            return false;   // side-effectful
        if (sym->Options & DBG_DUMP_COPY_TYPE_DATA)     return false;   // reads target memory
        if (sym->Options & DBG_DUMP_READ_PHYSICAL)      return false;
        if (sym->Options & DBG_DUMP_CALL_FOR_EACH)      return false;
        if (sym->Options & DBG_DUMP_LIST)               return false;   // list walk - not deterministic
        if (sym->Options & DBG_DUMP_ARRAY)              return false;   // varies per caller's addr
        if (sym->addr != 0)                             return false;   // caller wants data at addr
        if (sym->BufferSize != 0)                       return false;
        if (sym->Context != nullptr)                    return false;

        if (sym->nFields && sym->Fields) {
            for (ULONG i = 0; i < sym->nFields; ++i) {
                const FIELD_INFO& f = sym->Fields[i];
                if (f.fOptions & DBG_DUMP_FIELD_COPY_FIELD_DATA) return false;
                if (f.fOptions & DBG_DUMP_FIELD_CALL_BEFORE_PRINT) return false;
                if (f.fOptions & DBG_DUMP_FIELD_RETURN_ADDRESS)  return false;
                if (f.pBuffer != nullptr)                         return false;
            }
        }
        return true;
    }

    // ---------------------------------------------------------------------
    // Per-type cache handlers
    // ---------------------------------------------------------------------
    ULONG handle_get_type_size(PSYM_DUMP_PARAM sym, ULONG cb_size)
    {
        if (!sym || !sym->sName) return original_ioctl()(IG_GET_TYPE_SIZE, sym, cb_size);

        std::string key = build_type_size_key(sym);
        auto it = g_type_size_cache.find(key);
        if (it != g_type_size_cache.end()) {
            return it->second.return_code;
        }

        ULONG rc = original_ioctl()(IG_GET_TYPE_SIZE, sym, cb_size);
        g_type_size_cache.emplace(std::move(key), TypeSizeValue{ rc });
        return rc;
    }

    ULONG handle_dump_symbol_info(PSYM_DUMP_PARAM sym, ULONG cb_size)
    {
        if (!is_dump_symbol_cacheable(sym)) {
            return original_ioctl()(IG_DUMP_SYMBOL_INFO, sym, cb_size);
        }

        std::string key = build_dump_symbol_key(sym);
        auto it = g_dump_symbol_cache.find(key);
        if (it != g_dump_symbol_cache.end()) {
            const DumpSymbolValue& v = it->second;
            sym->ModBase   = v.mod_base;
            sym->TypeId    = v.type_id;
            sym->TypeSize  = v.type_size;
            sym->fPointer  = v.f_pointer;
            sym->fArray    = v.f_array;
            sym->fStruct   = v.f_struct;
            sym->fConstant = v.f_constant;
            if (sym->nFields && sym->Fields && v.fields.size() == sym->nFields) {
                for (ULONG i = 0; i < sym->nFields; ++i) {
                    const FieldOutput& src = v.fields[i];
                    FIELD_INFO&        dst = sym->Fields[i];
                    dst.size         = src.size;
                    dst.address      = src.address;
                    dst.TypeId       = src.type_id;
                    dst.FieldOffset  = src.field_offset;
                    dst.BufferSize   = src.buffer_size;
                    dst.BitField.Position = src.bit_pos;
                    dst.BitField.Size     = src.bit_size;
                    dst.fPointer     = src.f_pointer;
                    dst.fArray       = src.f_array;
                    dst.fStruct      = src.f_struct;
                    dst.fConstant    = src.f_constant;
                    dst.fStatic      = src.f_static;
                }
            }
            return v.return_code;
        }

        ULONG rc = original_ioctl()(IG_DUMP_SYMBOL_INFO, sym, cb_size);

        DumpSymbolValue v;
        v.return_code = rc;
        v.mod_base    = sym->ModBase;
        v.type_id     = sym->TypeId;
        v.type_size   = sym->TypeSize;
        v.f_pointer   = sym->fPointer;
        v.f_array     = sym->fArray;
        v.f_struct    = sym->fStruct;
        v.f_constant  = sym->fConstant;
        if (sym->nFields && sym->Fields) {
            v.fields.reserve(sym->nFields);
            for (ULONG i = 0; i < sym->nFields; ++i) {
                const FIELD_INFO& f = sym->Fields[i];
                v.fields.push_back(FieldOutput{
                    f.size, f.address, f.TypeId, f.FieldOffset, f.BufferSize,
                    f.BitField.Position, f.BitField.Size,
                    f.fPointer, f.fArray, f.fStruct, f.fConstant, f.fStatic
                });
            }
        }

        g_dump_symbol_cache.emplace(std::move(key), std::move(v));
        return rc;
    }

    // IG_GET_EXPRESSION_EX layout (from wdbgexts.h):
    // struct { PCSTR Expression; ULONG64 Value; ULONG64 Remainder; }
    struct GetExpressionEx {
        PCSTR    Expression;
        ULONG64  Value;
        ULONG64  Remainder;
    };

    ULONG handle_get_expression_ex(void* data, ULONG cb_size)
    {
        auto* p = static_cast<GetExpressionEx*>(data);
        if (!p || !p->Expression || cb_size < sizeof(GetExpressionEx)) {
            return original_ioctl()(IG_GET_EXPRESSION_EX, data, cb_size);
        }

        std::string key = p->Expression;
        auto it = g_expression_cache.find(key);
        if (it != g_expression_cache.end()) {
            p->Value     = it->second.value;
            p->Remainder = it->second.remainder;
            return it->second.return_code;
        }

        ULONG rc = original_ioctl()(IG_GET_EXPRESSION_EX, data, cb_size);
        g_expression_cache.emplace(std::move(key),
            ExpressionValue{ rc, p->Value, p->Remainder });
        return rc;
    }

    // ---------------------------------------------------------------------
    // The detour entry point.
    // Must have the exact WDBGAPI signature of PWINDBG_IOCTL_ROUTINE.
    // ---------------------------------------------------------------------
    ULONG WDBGAPI hooked_ioctl(USHORT IoctlType, PVOID lpvData, ULONG cbSizeOfContext)
    {
        if (!g_trampoline) {
            // Defensive - should never hit since install() verifies this.
            return 0;
        }

        switch (IoctlType)
        {
        case IG_GET_TYPE_SIZE:
            return handle_get_type_size(static_cast<PSYM_DUMP_PARAM>(lpvData), cbSizeOfContext);

        case IG_DUMP_SYMBOL_INFO:
            return handle_dump_symbol_info(static_cast<PSYM_DUMP_PARAM>(lpvData), cbSizeOfContext);

        case IG_GET_EXPRESSION_EX:
            return handle_get_expression_ex(lpvData, cbSizeOfContext);

        default:
            return original_ioctl()(IoctlType, lpvData, cbSizeOfContext);
        }
    }

    // ---------------------------------------------------------------------
    // Resolve the real Ioctl address by asking a fresh dbgeng client.
    // This does not need a loaded target - GetWindbgExtensionApis64 just
    // fills in the callback table with pointers into dbgeng.
    // ---------------------------------------------------------------------
    PWINDBG_IOCTL_ROUTINE resolve_real_ioctl()
    {
        IDebugClient*   client = nullptr;
        IDebugControl3* ctrl   = nullptr;
        PWINDBG_IOCTL_ROUTINE ioctl = nullptr;

        HRESULT hr = DebugCreate(__uuidof(IDebugClient), reinterpret_cast<void**>(&client));
        if (FAILED(hr) || !client) return nullptr;

        hr = client->QueryInterface(__uuidof(IDebugControl3), reinterpret_cast<void**>(&ctrl));
        if (SUCCEEDED(hr) && ctrl) {
            WINDBG_EXTENSION_APIS64 apis{};
            apis.nSize = sizeof(apis);
            hr = ctrl->GetWindbgExtensionApis64(&apis);
            if (SUCCEEDED(hr)) {
                ioctl = apis.lpIoctlRoutine;
            }
            ctrl->Release();
        }
        client->Release();
        return ioctl;
    }
}


namespace IoctlHook
{
    bool install()
    {
        if (g_installed.load(std::memory_order_acquire)) {
            return false;
        }

        PWINDBG_IOCTL_ROUTINE real = resolve_real_ioctl();
        if (!real) {
            return false;
        }

        g_detour = std::make_unique<PLH::x64Detour>(
            reinterpret_cast<uint64_t>(real),
            reinterpret_cast<uint64_t>(&hooked_ioctl),
            &g_trampoline);

        if (!g_detour->hook() || g_trampoline == 0) {
            g_detour.reset();
            g_trampoline = 0;
            return false;
        }

        // Pre-size caches so the hot loop never pays rehash cost.
        g_type_size_cache.reserve(512);
        g_expression_cache.reserve(512);
        g_dump_symbol_cache.reserve(2048);

        g_installed.store(true, std::memory_order_release);
        return true;
    }

    void uninstall()
    {
        if (!g_installed.load(std::memory_order_acquire)) {
            return;
        }
        if (g_detour) {
            g_detour->unHook();
            g_detour.reset();
        }
        g_trampoline = 0;
        invalidate_cache();
        g_installed.store(false, std::memory_order_release);
    }

    void invalidate_cache()
    {
        g_type_size_cache.clear();
        g_expression_cache.clear();
        g_dump_symbol_cache.clear();
    }

    bool is_installed()
    {
        return g_installed.load(std::memory_order_acquire);
    }
}
