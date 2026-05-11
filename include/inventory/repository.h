#pragma once

#include "inventory/types.h"

#include <vector>

namespace ip_inv {

struct AddToPoolResult {
    InventoryStatus status;
    std::vector<IpAddress> failedIps;

    [[nodiscard]] bool success() const noexcept {
        return status.success();
    }
};

class IpInventoryRepository {
public:
    virtual ~IpInventoryRepository() noexcept = default;
    virtual AddToPoolResult addIpAddresses(const std::vector<IpAddress>& addresses) = 0;
    // virtual ReserveIpResult reserveIp(std::string service_id, IpTypeSelection type) = 0;
    // virtual Status assignIps(std::string service_id, std::vector<std::string> ips) = 0;
    // virtual Status terminateIps(std::string service_id, std::vector<std::string> ips) = 0;
    // virtual Status changeServiceId(std::string old_service_id, std::string new_service_id) = 0;
    // virtual LookupResult lookupService(std::string service_id) const = 0;
};

} // namespace ip_inv
