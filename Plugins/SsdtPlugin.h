#pragma once
#include "IPlugin.h"

class SsdtPlugin : public IPlugin {
public:
    std::string_view name()        const noexcept override { return "ssdt"; }
    std::string_view description() const noexcept override { return "Dump SSDT (System Service Descriptor Table) with resolved symbol names"; }
    void run(DebugMagic& dbg) override;
};
