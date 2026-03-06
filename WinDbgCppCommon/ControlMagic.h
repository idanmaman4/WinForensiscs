#pragma once

#include "Types.h"
#include "DebugBridge.h"


class ControlMagic
{
public:
	ControlMagic(MasterDebugBridge& master_interface); 

	void wait_for_debug_event();


private:
	DebugBridge<IDebugControl> m_control;
};

