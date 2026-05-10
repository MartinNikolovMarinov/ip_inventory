#include "inventory/validation.h"

#include <cctype>
#include <string>

namespace ip_inv {
namespace {

bool isDecimalByte(const std::string& value) {
    if (value.empty() || value.size() > 3) {
        return false;
    }

    int parsed = 0;
    for (const char ch : value) {
        if (!std::isdigit(static_cast<unsigned char>(ch))) {
            return false;
        }

        parsed = (parsed * 10) + (ch - '0');
    }

    return parsed <= 255;
}

bool isIpv4Address(const std::string& value) {
    std::string::size_type partStart = 0;
    std::string::size_type partCount = 0;

    while (partStart <= value.size()) {
        const std::string::size_type partEnd = value.find('.', partStart);
        const std::string part = value.substr(partStart, partEnd - partStart);
        if (!isDecimalByte(part)) {
            return false;
        }

        ++partCount;
        if (partEnd == std::string::npos) {
            break;
        }

        partStart = partEnd + 1;
    }

    return partCount == 4;
}

bool countIpv6Groups(const std::string& value, std::string::size_type& groupCount) {
    if (value.empty()) {
        return true;
    }

    std::string::size_type groupStart = 0;
    while (groupStart <= value.size()) {
        const std::string::size_type groupEnd = value.find(':', groupStart);
        const std::string group = value.substr(groupStart, groupEnd - groupStart);

        if (group.empty() || group.size() > 4) {
            return false;
        }

        for (const char ch : group) {
            if (!std::isxdigit(static_cast<unsigned char>(ch))) {
                return false;
            }
        }

        ++groupCount;
        if (groupEnd == std::string::npos) {
            break;
        }

        groupStart = groupEnd + 1;
    }

    return true;
}

bool isIpv6Address(const std::string& value) {
    if (value.empty() || value.find(':') == std::string::npos) {
        return false;
    }

    std::string::size_type groupCount = 0;
    const std::string::size_type compression = value.find("::");
    if (compression == std::string::npos) {
        return countIpv6Groups(value, groupCount) && groupCount == 8;
    }

    if (value.find("::", compression + 2) != std::string::npos) {
        return false;
    }

    const std::string left = value.substr(0, compression);
    const std::string right = value.substr(compression + 2);
    return countIpv6Groups(left, groupCount) && countIpv6Groups(right, groupCount) && groupCount < 8;
}

} // namespace

bool isValidForType(const IpAddress& address) {
    switch (address.type) {
        case IpType::IPv4:
            return isIpv4Address(address.value);
        case IpType::IPv6:
            return isIpv6Address(address.value);
    }

    return false;
}

} // namespace ip_inv
