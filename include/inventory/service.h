#pragma once

#include "inventory/inventory_types.h"
#include "inventory/repository.h"

#include <memory>

namespace ip_inv {

class IpInventoryService {
public:
    explicit IpInventoryService(std::unique_ptr<IpInventoryRepository> repository, usize reservationExpirationSeconds);

    [[nodiscard]] AddToPoolResult addIpAddresses(std::vector<IpAddress>&& addresses);
    [[nodiscard]] ReserveIpResult reserveIpAddress(const std::string& serviceId, IpTypeSelection ipTypeSelection);

private:
    std::unique_ptr<IpInventoryRepository> m_repository;
    usize m_reservationExpirationSeconds = 0;
};

} // namespace ip_inv
