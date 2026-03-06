#pragma once

#include <string>
#include "Types.h"
#include "DebugBridge.h"

class MemoryMagic
{
public:
    MemoryMagic(MasterDebugBridge& master_interaface);

    Expected<Bytes> read_memory_virtual(Address address, size_t size);
  
    Expected<Bytes> read_memory_physical(Address address, size_t size);
  
    template <typename T>
    Expected<T> read_struct_memory_virtual(Address address);                                         

    Expected<Address> read_pointer_memory_virtual(Address address);
  
    template <typename T>
    Expected<T> read_struct_memory_physical(Address address);

    template <typename T>
    Expected<std::vector<T>> read_struct_array_memory_physical(Address address, size_t length);

    template <typename T>
    Expected<std::vector<T>> read_struct_array_memory_virtual(Address address, size_t length);


    Expected<Address> read_pointer_memory_physical(Address address);

private: 
   DebugBridge<IDebugDataSpaces> m_data_space;
};


template<typename T>
inline Expected<T> MemoryMagic::read_struct_memory_virtual(Address address)
{
    Expected<Bytes> content = read_memory_virtual(address, sizeof(T));
    if (content.has_value()) {
        return *reinterpret_cast<T*>(content->data());
    }
    
    return std::unexpected(content.error());
}


template<typename T>
inline Expected<T> MemoryMagic::read_struct_memory_physical(Address address)
{
    Expected<Bytes> content = read_memory_physical(address, sizeof(T));
    if (content.has_value()) {
        return *reinterpret_cast<T*>(content->data());
    }
    
    return std::unexpected(content.error());
}


template<typename T>
inline Expected<std::vector<T>> MemoryMagic::read_struct_array_memory_physical(Address address, size_t length)
{
    std::vector<T> result;
    for (size_t i = 0; i < length; i++)
    {
        Expected<T> value = read_struct_memory_physical<T>(address + i * sizeof(T));
        if (!value.has_value()) {
            return std::unexpected(value.error());
        }
        result.emplace_back(value.value());
    }

    return result;
}


template<typename T>
inline Expected<std::vector<T>> MemoryMagic::read_struct_array_memory_virtual(Address address, size_t length)
{
    std::vector<T> result;
    for (size_t i = 0; i < length; i++)
    {
        Expected<T> value = read_struct_memory_virtual<T>(address + i * sizeof(T));
        if (!value.has_value()) {
            return std::unexpected(value.error());
        }
        result.emplace_back(value.value());
    }

    return result;
}