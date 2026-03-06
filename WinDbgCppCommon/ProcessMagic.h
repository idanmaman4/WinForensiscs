#pragma once
#include "DebugMagic.h"

class ProcessMagic{
public:
    explicit ProcessMagic(DebugMagic& debug_magic, Address eprocess_address)
        : m_debug_magic(debug_magic)
    {
        Expected<void> attach = debug_magic.system().attach_to_process(eprocess_address);
        if (!attach.has_value())
        {
				throw attach.error();
        }
    }

    ~ProcessMagic()
    {
        try {
            ;

            m_debug_magic.system().attach_to_process(SYSTEM_PID);
        }
        catch (...){}
    }

    // no copy, no move — context is tied to this scope
    ProcessMagic(const ProcessMagic&)            = delete;
    ProcessMagic& operator=(const ProcessMagic&) = delete;
    ProcessMagic(ProcessMagic&&)                 = delete;
    ProcessMagic& operator=(ProcessMagic&&)      = delete;

private:
    static constexpr uint64_t SYSTEM_PID = 4;
    DebugMagic& m_debug_magic;
};

