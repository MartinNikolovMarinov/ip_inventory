#pragma once

#include "types.h"

#include <string>

namespace ip_inv {

[[nodiscard]] bool isValidDatabaseName(const std::string& name);
[[nodiscard]] bool isValidPort(i32 port);

} // namespace ip_inv
