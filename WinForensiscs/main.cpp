#include <windows.h>
#include <dbgeng.h>
#include <iostream>
#include <string>

#include "DebugMagic.h"
#include "PluginRegistry.h"

#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "dbgeng.lib")

static void print_usage(const wchar_t* exe) {
    std::wcerr <<
        L"Usage:\n"
        L"  " << exe << L" list\n"
        L"  " << exe << L" <dump.dmp> <plugin>\n\n"
        L"Run 'list' with no dump to see available plugins.\n\n"
        L"Examples:\n"
        L"  " << exe << L" list\n"
        L"  " << exe << L" C:\\Dumps\\MEMORY.DMP ssdt\n"
        L"  " << exe << L" C:\\Dumps\\MEMORY.DMP pslist\n"
        L"  " << exe << L" C:\\Dumps\\MEMORY.DMP vad\n";
}

// Print all registered plugins without loading a dump.
static void print_plugin_list(const PluginRegistry& registry) {
    std::cout << "\nAvailable plugins:\n";
    std::cout << "  " << std::string(50, '-') << "\n";
    for (const auto& [name, desc] : registry.get_plugin_info())
        std::cout << "  " << name << "\n    " << desc << "\n";
}

int wmain(int argc, wchar_t* argv[]) {
    PluginRegistry registry = make_plugin_registry();

    // "list" requires no dump — works as a standalone first argument
    if (argc == 2 && std::wstring(argv[1]) == L"list") {
        print_plugin_list(registry);
        return 0;
    }

    // All other invocations need exactly: <dump> <plugin>
    if (argc != 3) {
        print_usage(argv[0]);
        return 1;
    }

    std::wstring dump_path  = argv[1];
    std::wstring wplugin    = argv[2];
    std::string  plugin_name(wplugin.begin(), wplugin.end());

    // Allow "list" with a dump path too — still skip loading
    if (plugin_name == "list") {
        print_plugin_list(registry);
        return 0;
    }

    std::cout << "[*] Opening: ";
    std::wcout << dump_path << L"\n";

    DebugMagic dbg(dump_path);
    dbg.load_ntos_symbols();
    std::cout << "[*] Symbols loaded.\n";

    registry.run_one(plugin_name, dbg);
    return 0;
}
