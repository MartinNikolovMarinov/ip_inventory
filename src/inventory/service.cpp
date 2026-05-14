#include "inventory/inventory_types.h"
#include "inventory/repository.h"
#include "inventory/service.h"

#include <chrono>

// TODO: Implement a strict validation layer for the input arguments.

namespace ip_inv {

IpInventoryService::IpInventoryService(std::unique_ptr<IpInventoryRepository> repository, usize reservationExpirationSeconds)
    : m_repository(std::move(repository)),
      m_reservationExpirationSeconds(reservationExpirationSeconds) {}

//======================================================================================================================
// PUBLIC
//======================================================================================================================

InventoryStatus IpInventoryService::addIpAddresses(std::vector<IpAddress>&& addresses) {
    return m_repository->addIpAddresses(std::move(addresses));
}

ReserveIpResult IpInventoryService::reserveIpAddress(const std::string& serviceId, IpTypeSelection ipTypeSelection) {
    using namespace std::chrono;
    const i64 expirationTime = i64(system_clock::to_time_t(system_clock::now() + seconds(m_reservationExpirationSeconds)));
    return m_repository->reserveIpAddress(serviceId, ipTypeSelection, expirationTime);
}

InventoryStatus IpInventoryService::assignIpAddress(const std::string& serviceId, std::vector<IpAddress>&& addresses) {
    return m_repository->assignIpAddress(serviceId, std::move(addresses));
}

InventoryStatus IpInventoryService::terminateIpAssignment(const std::string& serviceId, std::vector<IpAddress>&& addresses) {
    return m_repository->terminateIpAssignment(serviceId, std::move(addresses));
}

InventoryStatus IpInventoryService::changeServiceId(const std::string& servideIdOld, const std::string& serviceIdNew) {
    return m_repository->changeServiceId(servideIdOld, serviceIdNew);
}

ServiceIpsResult IpInventoryService::getAssignedIpsForService(const std::string& servideId) {
    return m_repository->getAssignedIpsForService(servideId);
}

ReservedIpsResult IpInventoryService::getReservedIps() {
    return m_repository->getReservedIps();
}

void IpInventoryService::clearExpiredReservations() {
    m_repository->clearExpiredReservations();
}

} // namespace ip_inv
