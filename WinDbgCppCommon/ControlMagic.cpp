#include "ControlMagic.h"

ControlMagic::ControlMagic(MasterDebugBridge& master_interface) : m_control(master_interface)
{
}

void ControlMagic::wait_for_debug_event()
{
	HRESULT hr = m_control.get_interface()->WaitForEvent(DEBUG_WAIT_DEFAULT, INFINITE);
    if (FAILED(hr)) {
         throw std::exception("Debug Event Wait is invalid");
    }
}
