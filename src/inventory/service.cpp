#include "ip_utils.h"

#include "inventory/repository.h"
#include "inventory/service.h"

#include <chrono>

namespace ip_inv {

namespace {

AddToPoolResult parseIpAddresses(std::vector<IpAddress>& addresses);

} // namespace

IpInventoryService::IpInventoryService(std::unique_ptr<IpInventoryRepository> repository)
    : m_repository(std::move(repository)) {}

//======================================================================================================================
// PUBLIC
//======================================================================================================================

AddToPoolResult IpInventoryService::addIpAddresses(std::vector<IpAddress>&& addresses) {
    AddToPoolResult result = parseIpAddresses(addresses);

    if (!result.success()) {
        return result;
    }

    return m_repository->addIpAddresses(std::move(addresses));
}

ReserveIpResult IpInventoryService::reserveIpAddress(const std::string& serviceId, IpType ipType) {
    using namespace std::chrono;
    ReserveIpResult result;
    const i64 expirationTime = i64(system_clock::to_time_t(system_clock::now() + minutes(20))); // TODO: allow expiration time to be configurable

    const IpTypeSelection ipTypeSection = ipType == IpType::IPv4 ? IpTypeSelection::IPv4 : IpTypeSelection::IPv6;
    return m_repository->reserveIpAddress(serviceId, ipTypeSection, expirationTime);
}

//======================================================================================================================
// Internal Helper Functions
//======================================================================================================================

namespace {

AddToPoolResult parseIpAddresses(std::vector<IpAddress>& addresses) {
    AddToPoolResult result;

    if (addresses.empty()) {
        result.status.error = InventoryError::EmptyInput;
        result.status.detail = "Failed to add ip address; reason: empty input addresses list";
        return result;
    }

    for (auto& address : addresses) {
        bool parseOk = false;
        switch (address.type) {
            case IpType::IPv4:
                parseOk = parseIpV4(address);
                break;
            case IpType::IPv6:
                parseOk = parseIpV6(address);
                break;
        }

        if (!parseOk) {
            result.status.error = InventoryError::InvalidIp;
            result.status.detail = "Failed to add ip address; reason: invalid ip for declared type";
            result.failedIps.push_back(address);
        }
    }

    return result;
}

} // namespace

} // namespace ip_inv
