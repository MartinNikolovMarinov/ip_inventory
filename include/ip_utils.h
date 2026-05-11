#pragma once

#include "inventory/inventory_types.h"

#include <string>

namespace ip_inv {

[[nodiscard]] bool parseIpV4(const std::string& ipStr, IpAddress& address);
[[nodiscard]] bool parseIpV6(const std::string& ipStr, IpAddress& address);

} // namespace ip_inv
