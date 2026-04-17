#include "DebugMagic.h"
#include "PeUtils.h"
#include "TypeMagic.h"
#include "IoctlHook.h"
#include <iostream>


using namespace std;


DebugMagic::DebugMagic(const std::wstring& dump_name) : m_master_bridge(),
                                                 m_client(m_master_bridge),
                                                 m_control(m_master_bridge),
                                                 m_symbols(m_master_bridge),
                                                 m_memory(m_master_bridge),
                                                 m_field_magic(*this),
                                                 m_system(m_master_bridge),
                                                 m_efn(m_master_bridge)
{
    // Install the Ioctl detour once per process.  The hook is a process-wide
    // singleton - later DebugMagic instances share the same trampoline and
    // symbol cache.  Failure here is non-fatal: we still function, just
    // without the Ioctl cache speedup.
    IoctlHook::install();

    m_client.open_dump(dump_name);
    m_control.wait_for_debug_event();
}


DebugMagic::~DebugMagic()
{
}


void DebugMagic::load_module_base_symbols(const Address module_base)
{
    Expected<CV_INFO_PDB70> pdb_info = PeUtils::get_pdb_info_for_pe(*this, module_base);
    if (!pdb_info.has_value()) {
        throw pdb_info.error();
    }

    m_symbol_client.download_pdb(pdb_info.value());
    m_symbols.reload_symbol_path(m_symbol_client.get_symbol_directory());
}


void DebugMagic::load_ntos_symbols()
{
    return load_module_symbols("nt");
}


void DebugMagic::load_module_symbols(const std::string& module_name)
{
     Expected<Address> module_base = m_symbols.get_module_base(module_name);
    if (module_base.has_value()) {
        load_module_base_symbols(module_base.value());
        
    }
}


FieldInfoMagic& DebugMagic::get_field_info_magic()
{
    return m_field_magic;
}

Expected<std::shared_ptr<GenericTypeContainer>> DebugMagic::from_ptr(std::shared_ptr<GenericTypeContainer> container, const std::string& field)
{
    const optional<TypedValue> field_val = container->get(field);
    if (!field_val.has_value()) {
        return unexpected(exception("Invalid field name"));
    }

    return TypeMagic::do_magic_pointer(*this, *field_val);
}

MemoryMagic& DebugMagic::memory()
{
    return m_memory;
}

SymbolMagic& DebugMagic::symbols()
{
    return m_symbols;
}

SystemMagic& DebugMagic::system()
{
	return m_system;
}

EfnMagic& DebugMagic::efn()
{
    return m_efn;
}

MasterDebugBridge& DebugMagic::master()
{
    return m_master_bridge;
}


Expected<shared_ptr<GenericTypeContainer>> DebugMagic::struct_magic(
    const std::string& module_name,
    const std::string& type,
    Address     address)
{
    return TypeMagic::do_type_magic(*this, address, type, module_name);
}
