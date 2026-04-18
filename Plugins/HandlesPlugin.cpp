#include "HandlesPlugin.h"
#include "ProcessUtils.h"
#include "ProcessMagic.h"

#include <iostream>

BOOLEAN CALLBACK HandlesPlugin::handle_cb(PKDEXT_HANDLE_INFORMATION info, ULONG /*flags*/, PVOID ctx) {
    auto* c = static_cast<HandleCtx*>(ctx);
    c->count++;

    std::cout << "      Handle 0x" << std::hex << info->Handle
              << "  Object 0x"     << info->Object
              << "  Body 0x"       << info->ObjectBody
              << "  Access 0x"     << info->GrantedAccess
              << "  Attr 0x"       << info->HandleAttributes
              << std::dec
              << (info->PagedOut ? "  [paged-out]" : "")
              << "\n";
    return TRUE;
}

void HandlesPlugin::run(DebugMagic& dbg) {
    std::cout << "\n=== Handle Enumeration ===\n";

    ProcessUtils::for_each_process(dbg, [&](auto eprocess) -> bool {
        auto pid  = eprocess->as_number_unsigned("UniqueProcessId").value_or(0);
        auto name = eprocess->as_string("ImageFileName").value_or("Unknown");

        std::cout << "  [Pid:" << pid << "  " << name << "]\n";

        try {
            ProcessMagic ctx(dbg, eprocess->address());

            HandleCtx hctx{};
            HRESULT hr = dbg.efn().enumerate_handles(
                eprocess->address(), 0, 0, handle_cb, &hctx);

            if (SUCCEEDED(hr))
                std::cout << "    -> " << hctx.count << " handle(s)\n";
            else
                std::cout << "    -> enumeration failed: 0x"
                          << std::hex << hr << std::dec << "\n";
        }
        catch (const std::exception& e) {
            std::cout << "    -> ERROR: " << e.what() << "\n";
        }

        return true;
    });
}
