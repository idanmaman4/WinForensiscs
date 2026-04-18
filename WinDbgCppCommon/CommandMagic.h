#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "Types.h"

class DebugMagic;

struct VadEntry {
    Address     vad_address;
    ULONG       level;
    ULONG64     starting_vpn;
    ULONG64     ending_vpn;
    ULONG64     commit_charge;
    ULONG       vad_type;       // 0=Private, 2=ImageMap(Exe), etc.
    ULONG       protection;     // 1=READONLY, 4=READWRITE, 7=EXECUTE_WRITECOPY, etc.
    bool        private_memory;
    std::string file_name;        // Windows path (starts with '\') for file-backed mappings
    std::string section_info;     // e.g. "Pagefile section, shared commit 0x6"
};

class CommandMagic
{
public:
    CommandMagic();

    // Walk the VAD tree for a given EPROCESS and return parsed entries.
    // Results are cached by eprocess_address for subsequent calls.
    Expected<std::vector<VadEntry>> parse_vad(DebugMagic& debug_magic, Address eprocess_address);

    template<typename ValueType>
    Expected<ValueType> parse_windbg_command_value(const std::string& command, ValueType (*parser)(std::string));

private:
    std::unordered_map<Address, std::vector<VadEntry>> m_vad_cache;
};


template<typename ValueType>
inline Expected<ValueType> CommandMagic::parse_windbg_command_value(const std::string& command,
                                                                  ValueType(*parser)(std::string))
{
    return Expected<ValueType>();
}
