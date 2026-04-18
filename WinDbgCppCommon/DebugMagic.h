#pragma once

#pragma once
#include "Types.h"             
#include "GenericTypeContainer.h" 
#include "FieldInfoMagic.h"     
#include "DebugBridge.h"       
#include "WinStructres.hpp"     
#include <DbgEng.h>             
#include <string>
#include <vector>
#include <expected>
#include <memory>
#include "SymbolClient.h"

#include "ControlMagic.h"
#include "MemoryMagic.h"
#include "SymbolMagic.h"
#include "CommandMagic.h"
#include "ClientMagic.h"
#include "SystemMagic.h"
#include "EfnMagic.h"

class DebugMagic
{
public:
    DebugMagic(const std::wstring& path);

    ~DebugMagic();

    
    void load_module_base_symbols(const Address module_base);

	void load_ntos_symbols();

	void load_module_symbols(const std::string& module_name); 
    
    template<typename ValueType>
    Expected<ValueType> get_symbol_address_as_struct(const std::string& module,
                                                     const std::string& symbol);
    
    template<typename ValueType>
    Expected<std::vector<ValueType>> get_symbol_address_as_struct_array(const std::string& module, 
                                                                        const std::string& symbol,
                                                                        size_t length);
                    
	template <typename ValueType>
	Expected<ValueType> get_struct_field(const std::string& module_name,
                                         const std::string& type,
                                         const std::string& field_name,
                                         Address address);
   

    Expected<std::shared_ptr<GenericTypeContainer>> struct_magic(const std::string& module_name,
                                                      const std::string& type,
                                                      const Address address);

 

    template <FixedString  MODULE_NAME, FixedString TYPE_NAME>
    Expected<std::shared_ptr<GenericTypeContainer>> get_symbol_address_as(const std::string& module,
                                                         const std::string& symbol);

    template <FixedString  MODULE_NAME, FixedString TYPE_NAME, FixedString FIELD_NAME>
    Expected<std::shared_ptr<GenericTypeContainer>> get_struct_from_field_as(Address field_address); 


    FieldInfoMagic& get_field_info_magic();

    Expected<std::shared_ptr<GenericTypeContainer>> from_ptr(std::shared_ptr<GenericTypeContainer> container, const std::string& field);

    MemoryMagic&    memory();
    SymbolMagic&    symbols();
    SystemMagic&    system();
    EfnMagic&       efn();
    CommandMagic&   command_magic();
    MasterDebugBridge& master();


private:

    SymbolClient m_symbol_client;
    MasterDebugBridge m_master_bridge;

    // clients...
    MemoryMagic    m_memory;
    SymbolMagic    m_symbols;
    FieldInfoMagic m_field_magic;
    ClientMagic    m_client;
    ControlMagic   m_control;
    SystemMagic    m_system;
    EfnMagic       m_efn;
    CommandMagic   m_command;
};


template<typename ValueType>
inline Expected<ValueType> DebugMagic::get_symbol_address_as_struct(const std::string& module, const std::string& symbol)
{
    Expected<Address> symbol_address = m_symbols.get_symbol_address(module, symbol);
    if (!symbol_address.has_value()) {
        return std::unexpected(symbol_address.error());
    }

    
    return m_memory.read_struct_memory_virtual<ValueType>(symbol_address.value());
}


template<typename ValueType>
inline Expected<std::vector<ValueType>> DebugMagic::get_symbol_address_as_struct_array(const std::string& module, const std::string& symbol, size_t length)
{
    Expected<Address> symbol_address = m_symbols.get_symbol_address(module, symbol);
    
    if (!symbol_address.has_value()) {
        return std::unexpected(symbol_address.error());
    }
    
    return m_memory.read_struct_array_memory_virtual<ValueType>(symbol_address.value(), length);
}

template <typename ValueType>
Expected<ValueType> DebugMagic::get_struct_field(const std::string& module_name, const std::string& type,
	const std::string& field_name, Address address)
{
	ValueType result = {0};
	Expected<ULONG> field_offset = m_symbols.get_field_offset(module_name, type, field_name);
    if (!field_offset.has_value())
    {
		return std::unexpected(field_offset.error());  
    }

    return m_memory.read_struct_memory_virtual<ValueType>(address + *field_offset);
}


template<FixedString  MODULE_NAME, FixedString  TYPE_NAME>
inline Expected<std::shared_ptr<GenericTypeContainer>> DebugMagic::get_symbol_address_as(const std::string& module,
                                                                        const std::string& symbol)
{
    Expected<Address> sym_address = m_symbols.get_symbol_address(module, symbol);
    if (!sym_address.has_value()) {
        return std::unexpected(sym_address.error());
    }

    std::string module_name = MODULE_NAME.view().data();
    std::string type_name = TYPE_NAME.view().data();

    return struct_magic(module_name, type_name, sym_address.value());
}


template<FixedString MODULE_NAME, FixedString TYPE_NAME, FixedString FIELD_NAME>
inline Expected<std::shared_ptr<GenericTypeContainer>> DebugMagic::get_struct_from_field_as(Address field_address)
{
    std::string module_name = MODULE_NAME.view().data();
    std::string type_name = TYPE_NAME.view().data();
    std::string field_name = FIELD_NAME.view().data();

    Expected<Address> struct_address = m_symbols.get_struct_base_from_field(module_name, type_name, field_name, field_address);
    if (!struct_address.has_value()) {
        return std::unexpected(struct_address.error());
    }


     return struct_magic(module_name, type_name, struct_address.value());
}
