#include "PsListPlugin.h"
#include "ProcessUtils.h"
#include "MagicUtils.h"
#include "ProcessMagic.h"

#include <iostream>

void PsListPlugin::run(DebugMagic& dbg) {
    std::cout << "\n=== Process List ===\n";
    std::cout << "  Idx   PID    Name                  Full Path\n";
    std::cout << "  " << std::string(90, '-') << "\n";

    size_t idx = 0;
    ProcessUtils::for_each_process(dbg, [&](auto eprocess) -> bool {
        ++idx;

        auto pid  = eprocess->as_number_unsigned("UniqueProcessId").value_or(0);
        auto name = eprocess->as_string("ImageFileName").value_or("Unknown");

        std::wstring full_path;
        auto ifp = dbg.from_ptr(eprocess, "ImageFilePointer");
        if (ifp.has_value()) {
            auto us = ifp.value()->as_object("FileName");
            if (us.has_value())
                full_path = MagicUtils::parse_unicode_string(dbg, us.value());
        }

        std::cout << "  " << idx << ".)  Pid:" << pid
                  << "  Name:" << name;
        std::wcout << L"  Path:" << full_path << L"\n";

        // First loaded module from PEB
        try {
            ProcessMagic ctx(dbg, eprocess->address());
            auto peb = dbg.from_ptr(eprocess, "Peb");
            if (peb.has_value()) {
                auto ldr = dbg.from_ptr(peb.value(), "Ldr");
                if (ldr.has_value()) {
                    Address ldr_entry = ldr.value()
                        ->as_object("InLoadOrderModuleList").value()
                        ->as_pointer("Flink").value();

                    auto mod = dbg.get_struct_from_field_as<
                        "ntdll", "_LDR_DATA_TABLE_ENTRY", "InMemoryOrderLinks">(ldr_entry);
                    if (mod.has_value()) {
                        auto base_dll = mod.value()->as_object("BaseDllName");
                        if (base_dll.has_value()) {
                            std::wcout << L"       First PEB module: "
                                       << MagicUtils::parse_unicode_string(dbg, base_dll.value())
                                       << L"\n";
                        }
                    }
                }
            }
        }
        catch (...) {}

        return true;
    });

    std::cout << "  Total: " << idx << " process(es)\n";
}
