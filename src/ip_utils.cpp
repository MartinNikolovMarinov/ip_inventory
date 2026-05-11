#include "ip_utils.h"

#include <cstring>

#if defined(_WIN32)
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #define inet_pton InetPtonA
#else
    // Expect POSIX compliant OS
    #include <arpa/inet.h>
#endif

namespace ip_inv {

bool parseIpV4(const std::string& ipStr, IpAddress& address) {
    in_addr addr {};
    if (ipStr.length() >= INET_ADDRSTRLEN || inet_pton(AF_INET, ipStr.c_str(), &addr) != 1) {
        return false;
    }

    IpAddress parsed {};
    parsed.value = ipStr;
    parsed.type = IpType::IPv4;
    std::memcpy(parsed.bytes, &addr, sizeof(addr));

    address = parsed;
    return true;
}

bool parseIpV6(const std::string& ipStr, IpAddress& address) {
    in6_addr addr {};
    if (ipStr.length() >= INET6_ADDRSTRLEN || inet_pton(AF_INET6, ipStr.c_str(), &addr) != 1) {
        return false;
    }

    IpAddress parsed {};
    parsed.value = ipStr;
    parsed.type = IpType::IPv6;
    std::memcpy(parsed.bytes, &addr, sizeof(parsed.bytes));

    address = parsed;
    return true;
}

} // namespace ip_inv
