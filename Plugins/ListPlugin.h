#pragma once
#include "IPlugin.h"
#include <vector>
#include <utility>

class ListPlugin : public IPlugin {
public:
    // Constructed with a snapshot of all other registered plugins' names + descriptions.
    explicit ListPlugin(std::vector<std::pair<std::string, std::string>> entries);

    std::string_view name()        const noexcept override { return "list"; }
    std::string_view description() const noexcept override { return "List all available plugins"; }

    // DebugMagic is not used — dump loading is skipped for this plugin.
    void run(DebugMagic&) override;

private:
    std::vector<std::pair<std::string, std::string>> m_entries;
};
