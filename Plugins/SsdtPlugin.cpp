#include "SsdtPlugin.h"

#include <iostream>
#include <ranges>

void SsdtPlugin::run(DebugMagic& dbg) {
    std::cout << "\n=== SSDT (System Service Descriptor Table) ===\n";

    auto ssdt_size = dbg.get_symbol_address_as_struct<uint32_t>("nt", "KiServiceLimit");
    if (!ssdt_size.has_value()) {
        std::cout << "  [!] Failed to read KiServiceLimit\n";
        return;
    }

    auto ssdt = dbg.get_symbol_address_as_struct_array<uint32_t>("nt", "KiServiceTable", *ssdt_size);
    auto ssdt_address = dbg.symbols().get_symbol_address("nt", "KiServiceTable");

    if (!ssdt.has_value() || !ssdt_address.has_value()) {
        std::cout << "  [!] Failed to read KiServiceTable\n";
        return;
    }

    size_t i = 0;
    auto resolve = [&](uint32_t raw) -> std::pair<Address, std::string> {
        const Address entry_addr = (raw >> 4) + *ssdt_address;
        auto sym = dbg.symbols().get_symbol_from_address(entry_addr);
        return { entry_addr, sym ? sym->second : "Unknown" };
    };

    for (auto [addr, sym_name] : ssdt.value() | std::views::transform(resolve)) {
        std::cout << "  " << i++ << ".)  " << sym_name
                  << "  |  0x" << std::hex << addr << std::dec << "\n";
    }

    std::cout << "  Total entries: " << i << "\n";
}
