#include "validation.h"
#include "types.h"

namespace ip_inv {

bool isValidDatabaseName(const std::string& name) {
    if (name.empty() || name == "." || name == ".." || name.length() > 128) {
        return false;
    }

    for (char ch : name) {
        const bool isLetter = (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
        const bool isDigit = ch >= '0' && ch <= '9';
        const bool isAllowedSymbol = ch == '_' || ch == '-' || ch == '.';
        if (!isLetter && !isDigit && !isAllowedSymbol) {
            return false;
        }
    }

    return true;
}

bool isValidPort(i32 port) {
    bool ret = port > 0 && port < 65535;
    return ret;
}

bool isValidServiceId(const std::string& serviceId) {
    if (serviceId.empty() || serviceId.length() > 128) {
        return false;
    }

    for (char ch : serviceId) {
        const unsigned char byte = static_cast<unsigned char>(ch);
        if (byte < 32 || byte == 127) {
            return false;
        }
    }

    return true;
}

bool isValidIpTypeSelection(IpTypeSelection ipTypeSelection) {
    return ipTypeSelection == IpTypeSelection::IPv4
        || ipTypeSelection == IpTypeSelection::IPv6
        || ipTypeSelection == IpTypeSelection::Both;
}

bool isValidIpAddress(const IpAddress& address) {
    if (address.str.empty()) {
        return false;
    }

    const usize byteCount = IpAddress::byteCount(address.type);
    return byteCount == 4 || byteCount == 16;
}

bool isValidIpAddressList(const std::vector<IpAddress>& addresses) {
    if (addresses.empty()) {
        return false;
    }

    for (const IpAddress& address : addresses) {
        if (!isValidIpAddress(address)) {
            return false;
        }
    }

    return true;
}

} // namespace ip_inv
