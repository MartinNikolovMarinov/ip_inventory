#pragma once

#include "types.h"

#include <string>

namespace ip_inv {

enum struct HttpMethod {
    GET,
    POST,
    PATCH,
    PUT,
    DELETE
};

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

} // namespace ip_inv
