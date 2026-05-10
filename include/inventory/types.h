#pragma once

#include "types.h"

#include <string>
#include <vector>

namespace ip_inv {

enum struct IpType {
    IPv4,
    IPv6
};

enum struct IpTypeSelection {
    IPv4,
    IPv6,
    Both
};

enum struct IpState {
    Free,
    Reserved,
    Assigned
};

struct IpAddress {
    std::string value;
    IpType type;
};

enum struct InventoryError {
    None,
    EmptyInput,
    InvalidIp,
    DuplicateIp,
    TypeMismatch,
    IpAlreadyExists
};

struct InventoryStatus {
    InventoryError error = InventoryError::None;
    std::string detail;

    [[nodiscard]] bool success() const noexcept {
        return error == InventoryError::None;
    }
};

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
