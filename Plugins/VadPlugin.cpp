#include "VadPlugin.h"
#include "ProcessUtils.h"
#include "ProcessMagic.h"

#include <iostream>
#include <iomanip>

void VadPlugin::print_entries(const std::vector<VadEntry>& entries) {
    std::cout << "    VAD              Lvl  Start            End              Commit  Type            Protection        File / Section\n";

    for (const auto& e : entries) {
        const char* prot = [&]() -> const char* {
            switch (e.protection & 0x1F) {
                case 0:  return "---";
                case 1:  return "READONLY";
                case 2:  return "EXECUTE";
                case 3:  return "EXECUTE_READ";
                case 4:  return "READWRITE";
                case 5:  return "WRITECOPY";
                case 6:  return "EXECUTE_READWRITE";
                case 7:  return "EXECUTE_WRITECOPY";
                default: return "NO_ACCESS";
            }
        }();

        const char* type = [&]() -> const char* {
            if (e.private_memory) return "Private";
            switch (e.vad_type) {
                case 2:  return "Mapped Exe";
                case 3:  return "AWE";
                default: return "Mapped";
            }
        }();

        std::cout << "    0x" << std::hex << std::setw(16) << std::setfill('0') << e.vad_address
                  << "  "    << std::dec << std::setw(3)  << std::setfill(' ') << e.level
                  << "  "    << std::hex << std::setw(16) << std::setfill('0') << e.starting_vpn
                  << "  "    <<             std::setw(16) << std::setfill('0') << e.ending_vpn
                  << "  "    << std::dec << std::setw(6)  << std::setfill(' ') << e.commit_charge
                  << "  "    << std::setw(12) << type
                  << "  "    << std::setw(18) << prot;

        if (!e.file_name.empty())
            std::cout << "  " << e.file_name;
        else if (!e.section_info.empty())
            std::cout << "  [" << e.section_info << "]";

        std::cout << "\n";
    }
    std::cout << std::dec;
}

void VadPlugin::run(DebugMagic& dbg) {
    std::cout << "\n=== VAD (Virtual Address Descriptor) ===\n";

    ProcessUtils::for_each_process(dbg, [&](auto eprocess) -> bool {
        auto pid  = eprocess->as_number_unsigned("UniqueProcessId").value_or(0);
        auto name = eprocess->as_string("ImageFileName").value_or("Unknown");

        std::cout << "  [Pid:" << pid << "  " << name << "]\n";

        try {
            ProcessMagic ctx(dbg, eprocess->address());

            auto result = dbg.command_magic().parse_vad(dbg, eprocess->address());
            if (result.has_value()) {
                std::cout << "    VAD regions: " << result.value().size() << "\n";
                print_entries(result.value());
            } else {
                std::cout << "    VAD failed: " << result.error().what() << "\n";
            }
        }
        catch (const std::exception& e) {
            std::cout << "    ERROR: " << e.what() << "\n";
        }

        return true;
    });
}
