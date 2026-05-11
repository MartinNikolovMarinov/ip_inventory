#include "validation.h"
#include "types.h"

#if defined(_WIN32)
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #define inet_pton InetPtonA
#else
    // Expect POSIX compliant OS
    #include <arpa/inet.h>
#endif

// TODO: Write tests for the validations

namespace ip_inv {

bool isValidIpv4Address(const char* value, usize len) {
    // inet_pton expects the string to be null terminated!
    [[maybe_unused]] in_addr addr {};
    bool ret = len < INET_ADDRSTRLEN && inet_pton(AF_INET, value, &addr) == 1;
    return ret;
}

bool isValidIpv6Address(const char* value, usize len) {
    // inet_pton expects the string to be null terminated!
    [[maybe_unused]] in6_addr addr {};
    bool ret = len < INET6_ADDRSTRLEN && inet_pton(AF_INET6, value, &addr) == 1;
    return ret;
}


bool isValidIPAddress(const IpAddress& address) {
    bool ret = false;
    switch (address.type) {
        case IpType::IPv4:
            ret = isValidIpv4Address(address.value.data(), address.value.length());
            break;
        case IpType::IPv6:
            ret = isValidIpv6Address(address.value.data(), address.value.length());
            break;
    }

    return ret;
}

bool isValidPort(i32 port) {
    bool ret = port > 0 && port < 65535;
    return ret;
}

} // namespace ip_inv
