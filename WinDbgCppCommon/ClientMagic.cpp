#include "ClientMagic.h"

ClientMagic::ClientMagic(MasterDebugBridge& master_interfcace) : m_client(master_interfcace)
{
}

void ClientMagic::open_dump(const std::wstring& dump_name)
{
	HRESULT hr = m_client.get_interface()->OpenDumpFileWide(dump_name.c_str(), 0);

    if (FAILED(hr)) {
        throw std::exception("Couldnt initialize dump");
    }
}
