#pragma once

#include "types.h"

#include <string>
#include <vector>

namespace ip_inv {

enum struct IpType {
    IPv4 = 4,
    IPv6 = 6
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
    std::string str;
    u8 bytes[16] {};
    IpType type = IpType::IPv4;

    [[nodiscard]] static constexpr usize byteCount(IpType ipType) noexcept {
        switch (ipType) {
            case IpType::IPv4: return 4;
            case IpType::IPv6: return 16;
        }

        return 0;
    }

    [[nodiscard]] constexpr bool operator==(const IpAddress& other) const noexcept {
        if (type != other.type) {
            return false;
        }

        for (usize i = 0; i < byteCount(type); ++i) {
            if (bytes[i] != other.bytes[i]) {
                return false;
            }
        }

        return true;
    }

    [[nodiscard]] constexpr bool operator!=(const IpAddress& other) const noexcept {
        return !(*this == other);
    }
};

enum struct InventoryError {
    None,
    DbNotInitialized,
    DbError,
    InvalidIp,
    EmptyInput,
    ServiceNotFound,

    IpUnavailable
};

struct InventoryStatus {
    InventoryError error = InventoryError::None;
    std::string detail;

    [[nodiscard]] bool success() const noexcept {
        return error == InventoryError::None;
    }

    static constexpr inline InventoryStatus OkStatus() {
        return InventoryStatus {};
    }
};

struct ReserveIpResult {
    InventoryStatus status;
    std::vector<IpAddress> reservedIps;

    [[nodiscard]] bool success() const noexcept {
        return status.success();
    }
};

struct ServiceIpsResult {
    InventoryStatus status;
    std::vector<IpAddress> serviceIps;

    [[nodiscard]] bool success() const noexcept {
        return status.success();
    }
};

} // namespace ip_inv
