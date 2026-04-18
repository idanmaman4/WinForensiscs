#pragma once
#include "IPlugin.h"

#define KDEXT_64BIT
#include <wdbgexts.h>
#include "extsfns.h"

class HandlesPlugin : public IPlugin {
public:
    std::string_view name()        const noexcept override { return "handles"; }
    std::string_view description() const noexcept override { return "Enumerate open handles for every process"; }
    void run(DebugMagic& dbg) override;

private:
    struct HandleCtx { ULONG count = 0; };
    static BOOLEAN CALLBACK handle_cb(PKDEXT_HANDLE_INFORMATION info, ULONG flags, PVOID ctx);
};
