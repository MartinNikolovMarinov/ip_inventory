#include "ip_utils.h"

#include <cctype>
#include <cstring>
#include <string_view>

#if defined(_WIN32)
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #define inet_pton InetPtonA
#else
    // Expect POSIX compliant OS
    #include <arpa/inet.h>
#endif

namespace ip_inv {

namespace {

bool isStrictIpv4(std::string_view value);

} // namespace

//======================================================================================================================
// PUBLIC
//======================================================================================================================

bool parseIpV4(IpAddress& address) {
    in_addr addr {};

    if (!isStrictIpv4(address.str)) {
        return false;
    }
    if (address.str.length() >= INET_ADDRSTRLEN) {
        return false;
    }
    if (inet_pton(AF_INET, address.str.c_str(), &addr) != 1) {
        return false;
    }

    std::memcpy(address.bytes, &addr, sizeof(addr));

    return true;
}

bool parseIpV6(IpAddress& address) {
    in6_addr addr {};

    if (address.str.length() >= INET6_ADDRSTRLEN) {
        return false;
    }
    if (inet_pton(AF_INET6, address.str.c_str(), &addr) != 1) {
        return false;
    }

    std::memcpy(address.bytes, &addr, sizeof(address.bytes));

    return true;
}

//======================================================================================================================
// Internal helper functions
//======================================================================================================================

namespace {

// Keep IPv4 syntax policy deterministic across OS parsers. Some inet_pton implementations accept forms we do not want
// in inventory input, so this helper rejects them before inet_pton performs byte conversion.
//
// Accepted IPv4 form:
// - exactly 4 dot-separated segments
// - each segment is non-empty
// - each segment contains only decimal digits
// - each segment length is at most 3 characters
// - each segment value is in the inclusive range 0..255
// - no leading zeroes unless the segment is exactly "0"
//
// Examples:
// - accepted: "1.2.3.4", "0.0.0.0", "255.255.255.255"
// - rejected: "01.2.3.4", "1.2.3.256", "1.2.3.", "1.2.3.4:80"
bool isStrictIpv4(std::string_view value) {
    usize segmentCount = 0;
    usize segmentStart = 0;

    while (segmentStart <= value.length()) {
        const usize dot = value.find('.', segmentStart);
        const usize segmentEnd = dot == std::string_view::npos ? value.length() : dot;
        const usize segmentLength = segmentEnd - segmentStart;

        if (segmentLength == 0 || segmentLength > 3) {
            return false;
        }
        if (segmentLength > 1 && value[segmentStart] == '0') {
            // Segments prefixed with multiple zeros are not considered valid.
            return false;
        }

        i32 segmentValue = 0;
        for (usize i = segmentStart; i < segmentEnd; ++i) {
            const unsigned char ch = static_cast<unsigned char>(value[i]);
            if (!std::isdigit(ch)) {
                // Segments that don't start with a digit are not considered valid.
                return false;
            }
            segmentValue = (segmentValue * 10) + (value[i] - '0');
        }
        if (segmentValue > 255) {
            // Each segment should be in the range [0-255]
            return false;
        }

        segmentCount++;
        if (dot == std::string_view::npos) {
            break;
        }
        segmentStart = dot + 1;
    }

    return segmentCount == 4;
}

} // namespace

} // namespace ip_inv
