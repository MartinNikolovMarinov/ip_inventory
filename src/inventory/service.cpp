#include "ip_utils.h"

#include "inventory/repository.h"
#include "inventory/service.h"

namespace ip_inv {

IpInventoryService::IpInventoryService(std::unique_ptr<IpInventoryRepository> repository)
    : m_repository(std::move(repository)) {}

AddToPoolResult IpInventoryService::addIpAddresses(const std::vector<IpAddress>& addresses) {
    AddToPoolResult result;
    std::vector<IpAddress> parsedAddresses;
    parsedAddresses.reserve(addresses.size());

    if (addresses.empty()) {
        result.status.error = InventoryError::EmptyInput;
        result.status.detail = "Failed to add ip address; reason: empty input addresses list";
        return result;
    }

    for (const auto& address : addresses) {
        IpAddress parsedAddress = address;
        bool parseOk = false;
        switch (address.type) {
            case IpType::IPv4:
                parseOk = parseIpV4(address.value, parsedAddress);
                break;
            case IpType::IPv6:
                parseOk = parseIpV6(address.value, parsedAddress);
                break;
        }

        if (!parseOk) {
            result.status.error = InventoryError::InvalidIp;
            result.status.detail = "Failed to add ip address; reason: invalid ip for declared type";
            result.failedIps.push_back(address);
        }
        else {
            parsedAddresses.push_back(parsedAddress);
        }
    }

    if (!result.success()) {
        return result;
    }

    return m_repository->addIpAddresses(parsedAddresses);
}

} // namespace ip_inv
