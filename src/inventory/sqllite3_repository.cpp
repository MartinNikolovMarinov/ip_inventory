#include "inventory/sqllite3_repository.h"

namespace ip_inv {

IpInventoryRepositorySqlLite::~IpInventoryRepositorySqlLite() noexcept {}

AddToPoolResult IpInventoryRepositorySqlLite::addIpAddresses(const std::vector<IpAddress>&) {
    AddToPoolResult ret;
    // TODO: implement..
    return ret;
}

} // namespace ip_inv


