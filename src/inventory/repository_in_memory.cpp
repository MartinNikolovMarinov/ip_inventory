#include "inventory/repository_in_memory.h"

namespace ip_inv {

AddToPoolResult IpInventoryInMemory::addIpAddresses(const std::vector<IpAddress>& addresses) {
    AddToPoolResult result;

    std::lock_guard lock(m_mutex);
    for (const auto& addr : addresses) {
        m_ipPool.emplace(addr.value, addr);
    }

    return result;
}

} // namespace ip_inv
