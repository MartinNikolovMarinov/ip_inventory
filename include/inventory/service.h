#pragma once

#include "inventory/repository.h"

#include <memory>

namespace ip_inv {

class IpInventoryService {
public:
    explicit IpInventoryService(std::unique_ptr<IpInventoryRepository> repository);

    AddToPoolResult addIpAddresses(std::vector<IpAddress>&& addresses);

private:
    std::unique_ptr<IpInventoryRepository> m_repository;
};

} // namespace ip_inv
