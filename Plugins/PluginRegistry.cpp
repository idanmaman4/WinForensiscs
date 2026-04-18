#include "PluginRegistry.h"
#include "ListPlugin.h"
#include "SsdtPlugin.h"
#include "PsListPlugin.h"
#include "HandlesPlugin.h"
#include "VadPlugin.h"

#include <iostream>
#include <algorithm>

void PluginRegistry::register_plugin(std::unique_ptr<IPlugin> plugin) {
    m_plugins.push_back(std::move(plugin));
}

std::vector<std::pair<std::string, std::string>> PluginRegistry::get_plugin_info() const {
    std::vector<std::pair<std::string, std::string>> info;
    info.reserve(m_plugins.size());
    for (const auto& p : m_plugins)
        info.emplace_back(std::string(p->name()), std::string(p->description()));
    return info;
}

bool PluginRegistry::run_one(std::string_view name, DebugMagic& dbg) {
    auto it = std::ranges::find_if(m_plugins, [&](const auto& p) {
        return p->name() == name;
    });

    if (it == m_plugins.end()) {
        std::cerr << "[!] Unknown plugin: " << name << "\n"
                  << "    Run with 'list' to see available plugins.\n";
        return false;
    }

    try {
        (*it)->run(dbg);
    }
    catch (const std::exception& e) {
        std::cout << "[!] Plugin '" << name << "' threw: " << e.what() << "\n";
    }

    return true;
}

void PluginRegistry::run_all(DebugMagic& dbg) {
    for (const auto& p : m_plugins) {
        try {
            p->run(dbg);
        }
        catch (const std::exception& e) {
            std::cout << "[!] Plugin '" << p->name() << "' threw: " << e.what() << "\n";
        }
    }
}

PluginRegistry make_plugin_registry() {
    PluginRegistry reg;

    // Register all analysis plugins first
    reg.register_plugin(std::make_unique<SsdtPlugin>());
    reg.register_plugin(std::make_unique<PsListPlugin>());
    reg.register_plugin(std::make_unique<HandlesPlugin>());
    reg.register_plugin(std::make_unique<VadPlugin>());

    // ListPlugin snapshots the names+descriptions of every plugin registered so far
    reg.register_plugin(std::make_unique<ListPlugin>(reg.get_plugin_info()));

    return reg;
}
