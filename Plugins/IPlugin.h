#pragma once

#include "DebugMagic.h"
#include <string_view>

class IPlugin {
public:
    virtual ~IPlugin() = default;

    [[nodiscard]] virtual std::string_view name()        const noexcept = 0;
    [[nodiscard]] virtual std::string_view description() const noexcept = 0;

    virtual void run(DebugMagic& dbg) = 0;
};
