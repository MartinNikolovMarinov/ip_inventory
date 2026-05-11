#pragma once

#include "types.h"
#include "inventory/types.h"

namespace ip_inv {

[[nodiscard]] bool isValidIpv4Address(const char* value, usize len);
[[nodiscard]] bool isValidIpv6Address(const char* value, usize len);
[[nodiscard]] bool isValidIPAddress(const IpAddress& address);
[[nodiscard]] bool isValidPort(i32 port);

} // namespace ip_inv
