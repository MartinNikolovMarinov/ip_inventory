#pragma once

#include "inventory/repository.h"

namespace ip_inv {

class IpInventoryRepositorySqlLite : public IpInventoryRepository {
public:
    ~IpInventoryRepositorySqlLite() noexcept override;
    AddToPoolResult addIpAddresses(const std::vector<IpAddress>& addresses) override;
};

} // namespace ip_inv
