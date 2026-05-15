#pragma once

#include "inventory/inventory_types.h"

#include <memory>

namespace ip_inv {

class IpInventoryRepository;

class IpInventoryService {
public:
    explicit IpInventoryService(std::unique_ptr<IpInventoryRepository> repository, usize reservationExpirationSeconds);

    [[nodiscard]] InventoryStatus addIpAddresses(std::vector<IpAddress>&& addresses);
    [[nodiscard]] ReserveIpResult reserveIpAddress(const std::string& serviceId, IpTypeSelection ipTypeSelection);
    [[nodiscard]] InventoryStatus assignIpAddress(
        const std::string& serviceId,
        std::vector<IpAddress>&& addresses
    );
    [[nodiscard]] InventoryStatus terminateIpAssignment(
        const std::string& serviceId,
        std::vector<IpAddress>&& addresses
    );
    [[nodiscard]] InventoryStatus changeServiceId(const std::string& servideIdOld, const std::string& serviceIdNew);
    [[nodiscard]] IpAddressesResult getAvailableIps();
    [[nodiscard]] ServiceIpsResult getAssignedIpsForService(const std::string& servideId);
    [[nodiscard]] ReservedIpsResult getReservedIps();

    void clearExpiredReservations();

private:
    std::unique_ptr<IpInventoryRepository> m_repository;
    usize m_reservationExpirationSeconds = 0;
};

} // namespace ip_inv
