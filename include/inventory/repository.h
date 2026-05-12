#pragma once

#include "inventory/inventory_types.h"

#include <filesystem>
#include <vector>

namespace ip_inv {

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

    virtual void initializeDb(bool dropCreate = false, std::filesystem::path schemaInitScriptPath = {}) = 0;

    [[nodiscard]] virtual AddToPoolResult addIpAddresses(const std::vector<IpAddress>& addresses) = 0;
};

} // namespace ip_inv
