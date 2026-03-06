#pragma once

#include <string>
#include "Types.h"
#include "DebugMagic.h"


class CommandMagic
{
public:
    CommandMagic();

    template<typename ValueType>
    Expected<ValueType> parse_windbg_command_value(const std::string& command, ValueType (*parser)(std::string));

private:

};


template<typename ValueType>
inline Expected<ValueType> CommandMagic::parse_windbg_command_value(const std::string& command,
                                                                  ValueType(*parser)(std::string))
{
    return Expected<ValueType>();
}