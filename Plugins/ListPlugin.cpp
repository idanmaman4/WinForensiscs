#include "ListPlugin.h"
#include <iostream>

ListPlugin::ListPlugin(std::vector<std::pair<std::string, std::string>> entries)
    : m_entries(std::move(entries))
{
}

void ListPlugin::run(DebugMagic& /*dbg*/) {
    std::cout << "\nAvailable plugins:\n";
    std::cout << "  " << std::string(50, '-') << "\n";
    for (const auto& [name, desc] : m_entries)
        std::cout << "  " << name << "\n    " << desc << "\n";
}
