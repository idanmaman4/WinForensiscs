#pragma once
#include "IPlugin.h"
#include "CommandMagic.h"

class VadPlugin : public IPlugin {
public:
    std::string_view name()        const noexcept override { return "vad"; }
    std::string_view description() const noexcept override { return "Dump VAD (Virtual Address Descriptor) tree for every process"; }
    void run(DebugMagic& dbg) override;

private:
    static void print_entries(const std::vector<VadEntry>& entries);
};
