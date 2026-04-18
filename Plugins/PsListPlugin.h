#pragma once
#include "IPlugin.h"

class PsListPlugin : public IPlugin {
public:
    std::string_view name()        const noexcept override { return "pslist"; }
    std::string_view description() const noexcept override { return "List all processes (PID, name, full path, first PEB module)"; }
    void run(DebugMagic& dbg) override;
};
