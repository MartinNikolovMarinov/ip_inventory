#pragma once

#include "inventory/inventory_types.h"
#include "inventory/repository.h"

#include <memory>

namespace ip_inv {

class IpInventoryService {
public:
    explicit IpInventoryService(std::unique_ptr<IpInventoryRepository> repository);

    [[nodiscard]] AddToPoolResult addIpAddresses(std::vector<IpAddress>&& addresses);
    [[nodiscard]] ReserveIpResult reserveIpAddress(const std::string& serviceId, IpType ipType);

private:
    std::unique_ptr<IpInventoryRepository> m_repository;
};

} // namespace ip_inv
