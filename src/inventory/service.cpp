#include "inventory/service.h"
#include "inventory/validation.h"

namespace ip_inv {

InventoryService::InventoryService(IpInventoryRepository& repository)
    : m_repository(repository) {}

AddToPoolResult InventoryService::addIpAddresses(const std::vector<IpAddress>& addresses) {
    AddToPoolResult result;

    if (addresses.empty()) {
        result.status.error = InventoryError::EmptyInput;
        result.status.detail = "Failed to add ip address; reason: empty input addresses list";
        return result;
    }

    for (const auto& address : addresses) {
        if (!isValidForType(address)) {
            result.status.error = InventoryError::InvalidIp;
            result.status.detail = "Failed to add ip address; reason: invalid ip for declared type";
            result.failedIps.push_back(address);
        }
    }

    if (!result.success()) {
        return result;
    }

    return m_repository.addIpAddresses(addresses);
}

} // namespace ip_inv
