#include "validation.h"

#include "inventory/repository.h"
#include "inventory/service.h"

namespace ip_inv {

IpInventoryService::IpInventoryService(std::unique_ptr<IpInventoryRepository> repository)
    : m_repository(std::move(repository)) {}

AddToPoolResult IpInventoryService::addIpAddresses(const std::vector<IpAddress>& addresses) {
    AddToPoolResult result;

    if (addresses.empty()) {
        result.status.error = InventoryError::EmptyInput;
        result.status.detail = "Failed to add ip address; reason: empty input addresses list";
        return result;
    }

    for (const auto& address : addresses) {
        if (!isValidIPAddress(address)) {
            result.status.error = InventoryError::InvalidIp;
            result.status.detail = "Failed to add ip address; reason: invalid ip for declared type";
            result.failedIps.push_back(address);
        }
    }

    if (!result.success()) {
        return result;
    }

    return m_repository->addIpAddresses(addresses);
}

} // namespace ip_inv
