#pragma once

#include <string>
#include "Types.h"
#include "DebugBridge.h"
#include <filesystem>

class SymbolMagic
{
public:
    SymbolMagic(MasterDebugBridge& master_interface);

    void reload_symbol_path(std::filesystem::path symbol_path);

    
    Expected<Address> get_module_base(const std::string& module_name);
  
    Expected<Address> get_ntos_base();
	
    Expected<std::string> get_type_name(const ULONG64 mod, ULONG type_id);



    Expected<Address> get_symbol_address(const std::string& module,
                                        const std::string& symbol);

    
	Expected<std::vector<std::string>> get_field_names(const ULONG64 mod, ULONG container_type_id);
    
    
    Expected<ULONG> get_type_id(ULONG64 mod,
                                const std::string& type_name);
       
    Expected<ULONG> get_type_size(ULONG64 mod,
                                  ULONG type_id);

      
    Expected<FieldInfo> get_field_info(ULONG64 mod,
                                       ULONG container_type_id,
                                       const std::string& field_name);

    Expected<std::pair<std::string,std::string>> get_symbol_from_address(Address address);

    std::string format_symbol_module(const std::string& module, const std::string symbol);


    Expected<Address> get_struct_base_from_field(const std::string& module_name, 
                                                         const std::string& type,
                                                         const std::string& field_name, 
                                                         Address address);

    Expected<ULONG> get_field_offset(const std::string& module_name, const std::string& type_name, const std::string& field_name);
	
	Expected<ULONG> get_field_offset(Address module_base, ULONG type_id, const std::string& field_name);




private: 
    DebugBridge<IDebugSymbols3> m_symbols_fields;
    DebugBridge<IDebugSymbols5> m_symbols;
};

