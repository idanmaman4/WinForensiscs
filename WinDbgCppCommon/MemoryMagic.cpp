#include "MemoryMagic.h"

using namespace std;

MemoryMagic::MemoryMagic(MasterDebugBridge& master_interaface) : m_data_space(master_interaface)
{
}

Expected<Bytes> MemoryMagic::read_memory_virtual(Address address, size_t size)
{
    Bytes result(size);

    HRESULT hr = m_data_space.get_interface()->ReadVirtual(address, result.data(), result.size(), nullptr);
    if (FAILED(hr)) {
        return unexpected(std::exception("Couldn't read virtual memory"));
    }
    
    return result;
}

Expected<Bytes> MemoryMagic::read_memory_physical(Address address, size_t size)
{
    Bytes result(size);

    HRESULT hr = m_data_space.get_interface()->ReadPhysical(address, result.data(), result.size(), nullptr);
    if (FAILED(hr)) {
        return unexpected(std::exception("Couldn't read virtual memory"));
    }

    return result;
}

Expected<Address> MemoryMagic::read_pointer_memory_virtual(Address address)
{
    Address pointer;
    HRESULT hr = m_data_space.get_interface()->ReadPointersVirtual(1, address,&pointer);
    if (FAILED(hr)) {
        return unexpected(std::exception("Couldn't read virtual memory"));
    }

    return pointer;
}

Expected<Address> MemoryMagic::read_pointer_memory_physical(Address address)
{
    Expected<Bytes> res = read_memory_physical(address,sizeof(Address));
    if (res.has_value()) {
        return *reinterpret_cast<Address*>(res.value().data());
    }

    return unexpected(res.error());
}