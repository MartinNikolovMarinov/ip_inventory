#pragma once

#include "inventory/inventory_types.h"

#include <filesystem>
#include <vector>

namespace ip_inv {

class IpInventoryRepository {
public:
    virtual ~IpInventoryRepository() noexcept = default;

    virtual void initializeDb(bool dropCreate = false, std::filesystem::path schemaInitScriptPath = {}) = 0;

    [[nodiscard]] virtual InventoryStatus addIpAddresses(const std::vector<IpAddress>& addresses) = 0;
    [[nodiscard]] virtual ReserveIpResult reserveIpAddress(
        const std::string& serviceId,
        IpTypeSelection ipTypeSection,
        i64 expirationTime
    ) = 0;
    [[nodiscard]] virtual InventoryStatus assignIpAddress(
        const std::string& serviceId,
        std::vector<IpAddress>&& addresses
    ) = 0;
    [[nodiscard]] virtual InventoryStatus terminateIpAssignment(
        const std::string& serviceId,
        std::vector<IpAddress>&& addresses
    ) = 0;
    [[nodiscard]] virtual InventoryStatus changeServiceId(
        const std::string& servideIdOld,
        const std::string& serviceIdNew
    ) = 0;
    [[nodiscard]] virtual ServiceIpsResult getAssignedIpsForService(const std::string& servideId) = 0;
    [[nodiscard]] virtual ReservedIpsResult getReservedIps() = 0;

    virtual void clearExpiredReservations() = 0;
};

} // namespace ip_inv
