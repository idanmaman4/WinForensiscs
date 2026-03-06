#pragma once

#include "DebugBridge.h"
#include <string>
#include <vector>
#include "Types.h"


class SystemMagic
{
public: 
	SystemMagic(MasterDebugBridge& master_interface);

	Expected<void> attach_to_process(Address eprocess_address);

	Expected<Address> get_current_eprocess();

	Expected<std::string> get_current_process_name();

	Expected<Address> get_current_process_peb(); 

	Expected<void> attach_to_thread(ULONG thread_id);

	Expected<Address> get_current_thread_teb(); 

private:
	 DebugBridge<IDebugSystemObjects4> m_system;
};

