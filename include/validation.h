#pragma once

#include "inventory/inventory_types.h"
#include "types.h"

#include <string>
#include <vector>

namespace ip_inv {

[[nodiscard]] bool isValidDatabaseName(const std::string& name);
[[nodiscard]] bool isValidPort(i32 port);
[[nodiscard]] bool isValidServiceId(const std::string& serviceId);
[[nodiscard]] bool isValidIpTypeSelection(IpTypeSelection ipTypeSelection);
[[nodiscard]] bool isValidIpAddress(const IpAddress& address);
[[nodiscard]] bool isValidIpAddressList(const std::vector<IpAddress>& addresses);

} // namespace ip_inv
