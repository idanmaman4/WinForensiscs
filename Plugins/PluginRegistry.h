#pragma once

#include "IPlugin.h"
#include <memory>
#include <vector>
#include <string_view>

class PluginRegistry {
public:
    void register_plugin(std::unique_ptr<IPlugin> plugin);

    // Returns {name, description} for every registered plugin.
    std::vector<std::pair<std::string, std::string>> get_plugin_info() const;

    // Run a single plugin by name. Returns false if not found.
    // Plugins that don't need the dump (e.g. ListPlugin) ignore the dbg argument.
    bool run_one(std::string_view name, DebugMagic& dbg);

    // Run every registered plugin in registration order.
    void run_all(DebugMagic& dbg);

private:
    std::vector<std::unique_ptr<IPlugin>> m_plugins;
};

// Creates a PluginRegistry pre-loaded with every built-in plugin.
PluginRegistry make_plugin_registry();
