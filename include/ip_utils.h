#pragma once

#include "inventory/inventory_types.h"

namespace ip_inv {

[[nodiscard]] bool parseIpV4(IpAddress& address);
[[nodiscard]] bool parseIpV6(IpAddress& address);

} // namespace ip_inv
