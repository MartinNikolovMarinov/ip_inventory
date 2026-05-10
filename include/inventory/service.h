#pragma once

#include "inventory/types.h"

namespace ip_inv {

class InventoryService {
public:
    explicit InventoryService(IpInventoryRepository& repository);

    AddToPoolResult addIpAddresses(const std::vector<IpAddress>& addresses);

private:
    IpInventoryRepository& m_repository;
};

} // namespace ip_inv
