#include "inventory/inventory_types.h"
#include "ip_utils.h"

#include "inventory/repository.h"
#include "inventory/service.h"

#include <chrono>

namespace ip_inv {

namespace {

AddToPoolResult parseIpAddresses(std::vector<IpAddress>& addresses);

} // namespace

IpInventoryService::IpInventoryService(std::unique_ptr<IpInventoryRepository> repository, usize reservationExpirationSeconds)
    : m_repository(std::move(repository)),
      m_reservationExpirationSeconds(reservationExpirationSeconds) {}

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

ReserveIpResult IpInventoryService::reserveIpAddress(const std::string& serviceId, IpTypeSelection ipTypeSelection) {
    using namespace std::chrono;
    ReserveIpResult result;
    const i64 expirationTime = i64(system_clock::to_time_t(system_clock::now() + seconds(m_reservationExpirationSeconds)));
    return m_repository->reserveIpAddress(serviceId, ipTypeSelection, expirationTime);
}

void IpInventoryService::clearExpiredReservations() {
    m_repository->clearExpiredReservations();
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
        bool isParseOk = false;
        switch (address.type) {
            case IpType::IPv4:
                isParseOk = parseIpV4(address);
                break;
            case IpType::IPv6:
                isParseOk = parseIpV6(address);
                break;
        }

        if (!isParseOk) {
            result.status.error = InventoryError::InvalidIp;
            result.status.detail = "Failed to add ip address; reason: invalid ip for declared type";
        }
    }

    return result;
}

} // namespace

} // namespace ip_inv
