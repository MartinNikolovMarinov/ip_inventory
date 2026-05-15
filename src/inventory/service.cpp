#include "inventory/inventory_types.h"
#include "inventory/repository.h"
#include "inventory/service.h"
#include "validation.h"

#include <chrono>
#include <stdexcept>
#include <utility>

namespace ip_inv {

namespace {

InventoryStatus invalidArgumentStatus(std::string detail) {
    InventoryStatus status;
    status.error = InventoryError::InvalidArgument;
    status.detail = std::move(detail);
    return status;
}

} // namespace

IpInventoryService::IpInventoryService(std::unique_ptr<IpInventoryRepository> repository, usize reservationExpirationSeconds)
    : m_repository(std::move(repository)),
      m_reservationExpirationSeconds(reservationExpirationSeconds) {
    if (m_repository == nullptr) {
        throw std::invalid_argument("inventory service requires a repository");
    }
    if (m_reservationExpirationSeconds == 0) {
        throw std::invalid_argument("reservation expiration seconds must be greater than zero");
    }
}

//======================================================================================================================
// PUBLIC
//======================================================================================================================

InventoryStatus IpInventoryService::addIpAddresses(std::vector<IpAddress>&& addresses) {
    if (!isValidIpAddressList(addresses)) {
        return invalidArgumentStatus("Failed to add IP addresses; reason: invalid IP address list");
    }

    return m_repository->addIpAddresses(std::move(addresses));
}

ReserveIpResult IpInventoryService::reserveIpAddress(const std::string& serviceId, IpTypeSelection ipTypeSelection) {
    ReserveIpResult ret;

    if (!isValidServiceId(serviceId)) {
        ret.status = invalidArgumentStatus("Failed to reserve IP address; reason: invalid service id");
        return ret;
    }
    if (!isValidIpTypeSelection(ipTypeSelection)) {
        ret.status = invalidArgumentStatus("Failed to reserve IP address; reason: invalid ip type selection");
        return ret;
    }

    using namespace std::chrono;
    const i64 expirationTime = i64(system_clock::to_time_t(system_clock::now() + seconds(m_reservationExpirationSeconds)));
    return m_repository->reserveIpAddress(serviceId, ipTypeSelection, expirationTime);
}

InventoryStatus IpInventoryService::assignIpAddress(const std::string& serviceId, std::vector<IpAddress>&& addresses) {
    if (!isValidServiceId(serviceId)) {
        return invalidArgumentStatus("Failed to assign IP address; reason: invalid service id");
    }
    if (!isValidIpAddressList(addresses)) {
        return invalidArgumentStatus("Failed to assign IP address; reason: invalid IP address list");
    }

    return m_repository->assignIpAddress(serviceId, std::move(addresses));
}

InventoryStatus IpInventoryService::terminateIpAssignment(const std::string& serviceId, std::vector<IpAddress>&& addresses) {
    if (!isValidServiceId(serviceId)) {
        return invalidArgumentStatus("Failed to terminate IP address; reason: invalid service id");
    }
    if (!isValidIpAddressList(addresses)) {
        return invalidArgumentStatus("Failed to terminate IP address; reason: invalid IP address list");
    }

    return m_repository->terminateIpAssignment(serviceId, std::move(addresses));
}

InventoryStatus IpInventoryService::changeServiceId(const std::string& servideIdOld, const std::string& serviceIdNew) {
    if (!isValidServiceId(servideIdOld)) {
        return invalidArgumentStatus("Failed to change service id; reason: invalid old service id");
    }
    if (!isValidServiceId(serviceIdNew)) {
        return invalidArgumentStatus("Failed to change service id; reason: invalid new service id");
    }

    return m_repository->changeServiceId(servideIdOld, serviceIdNew);
}

IpAddressesResult IpInventoryService::getAvailableIps() {
    return m_repository->getAvailableIps();
}

ServiceIpsResult IpInventoryService::getAssignedIpsForService(const std::string& servideId) {
    ServiceIpsResult ret;

    if (!isValidServiceId(servideId)) {
        ret.status = invalidArgumentStatus("Failed to get assigned IP addresses for service; reason: invalid service id");
        return ret;
    }

    return m_repository->getAssignedIpsForService(servideId);
}

ReservedIpsResult IpInventoryService::getReservedIps() {
    return m_repository->getReservedIps();
}

void IpInventoryService::clearExpiredReservations() {
    m_repository->clearExpiredReservations();
}

} // namespace ip_inv
