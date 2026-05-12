#pragma once

#include <string>

namespace ip_inv {

constexpr bool eqIgnoreCase(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) {
        return false;
    }

    for (size_t i = 0; i < a.size(); ++i) {
        if (std::tolower(char(a[i])) != std::tolower(char(b[i]))) {
            return false;
        }
    }

    return true;
}

} // namespace ip_inv
