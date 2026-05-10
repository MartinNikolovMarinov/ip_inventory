#pragma once

#include "inventory/types.h"

#include <mutex>
#include <string>
#include <unordered_map>

namespace ip_inv {

class IpInventoryInMemory : public IpInventoryRepository {
public:
    AddToPoolResult addIpAddresses(const std::vector<IpAddress>& addresses) override;

private:
    mutable std::mutex m_mutex;
    std::unordered_map<std::string, IpAddress> m_ipPool;
};

} // namespace ip_inv
