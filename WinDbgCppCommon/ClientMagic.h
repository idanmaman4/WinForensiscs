#pragma once
#include "Types.h"
#include "DebugBridge.h"


class ClientMagic
{
public:
    ClientMagic(MasterDebugBridge& master_interfcace);

    void open_dump(const std::wstring& dump_name); 

private:
    DebugBridge<IDebugClient6> m_client;
};

