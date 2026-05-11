#include <unity.h>

#include "ip_utils.h"

using namespace ip_inv;

void setUp() {}

void tearDown() {}

static void parses_ipv4_addresses() {
    struct TestCase {
        const char* value;
        bool expected;
        IpAddress address;
    };

    const TestCase tests[] = {
        // Valid cases
        {"1.2.3.4", true, {"", {1, 2, 3, 4}, IpType::IPv4}},
        {"192.168.1.1", true, {"", {192, 168, 1, 1}, IpType::IPv4}},
        {"127.0.0.1", true, {"", {127, 0, 0, 1}, IpType::IPv4}},
        {"0.0.0.0", true, {"", {0, 0, 0, 0}, IpType::IPv4}},
        {"255.255.255.255", true, {"", {255, 255, 255, 255}, IpType::IPv4}},

        // Invalid cases
        {"01.2.3.4", false, {}},
        {"1.02.3.4", false, {}},
        {"1.2.3.04", false, {}},
        {"1.2.3.256", false, {}},
        {"256.1.1.1", false, {}},
        {"-1.2.3.4", false, {}},
        {"1.-2.3.4", false, {}},
        {"192.168.1", false, {}},
        {"1.2.3.", false, {}},
        {".1.2.3.4", false, {}},
        {"1..2.3", false, {}},
        {"192.168.1.1.1", false, {}},
        {"1.2.3.4\n", false, {}},
        {" 1.2.3.4", false, {}},
        {"1.2.3.4 ", false, {}},
        {"1.2.3.a", false, {}},
        {"1.2.3.4:80", false, {}},
        {"not-an-ip", false, {}},
        {"", false, {}},
    };

    for (const TestCase& test : tests) {
        IpAddress address {};
        const bool parsed = parseIpV4(test.value, address);
        TEST_ASSERT_EQUAL_MESSAGE(test.expected, parsed, test.value);
        if (parsed) {
            TEST_ASSERT_TRUE_MESSAGE(address == test.address, test.value);
        }
    }
}

static void parses_ipv6_addresses() {
    struct TestCase {
        const char* value;
        bool expected;
        IpAddress address;
    };

    const TestCase tests[] = {
        // Valid cases
        {"::", true, {"", {}, IpType::IPv6}},
        {"::1", true, {"", {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}, IpType::IPv6}},
        {"::ffff:192.168.1.1", true, {"", {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 192, 168, 1, 1}, IpType::IPv6}},
        {"2001:db8::1", true, {"", {0x20, 0x01, 0x0d, 0xb8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}, IpType::IPv6}},
        {"2001:0db8:0:0:0:0:0:1", true, {"", {0x20, 0x01, 0x0d, 0xb8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}, IpType::IPv6}},
        {"fe80::1", true, {"", {0xfe, 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}, IpType::IPv6}},
        {"2001:db8:0:0:0:0:2:1", true, {"", {0x20, 0x01, 0x0d, 0xb8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 1}, IpType::IPv6}},
        {"2001:db8::2:1", true, {"", {0x20, 0x01, 0x0d, 0xb8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 1}, IpType::IPv6}},
        {"2001:db8:85a3::8a2e:370:7334", true, {"", {0x20, 0x01, 0x0d, 0xb8, 0x85, 0xa3, 0, 0, 0, 0, 0x8a, 0x2e, 0x03, 0x70, 0x73, 0x34}, IpType::IPv6}},

        // Invalid cases
        {"2001:db8:0:0:0:0:2:1:9", false, {}},
        {"2001:db8::2::1", false, {}},
        {"2001:db8:", false, {}},
        {":2001:db8::1", false, {}},
        {"2001:db8::g", false, {}},
        {"2001:db8:::1", false, {}},
        {"12345::1", false, {}},
        {"2001:db8::1\n", false, {}},
        {" 2001:db8::1", false, {}},
        {"2001:db8::1 ", false, {}},
        {"[2001:db8::1]", false, {}},
        {"not-an-ip", false, {}},
        {"", false, {}},
    };

    for (const TestCase& test : tests) {
        IpAddress address {};
        const bool parsed = parseIpV6(test.value, address);
        TEST_ASSERT_EQUAL_MESSAGE(test.expected, parsed, test.value);
        if (parsed) {
            TEST_ASSERT_TRUE_MESSAGE(address == test.address, test.value);
        }
    }
}

static void compares_equivalent_ip_addresses() {
    IpAddress compressed {};
    TEST_ASSERT_TRUE(parseIpV6("2001:db8::1", compressed));

    IpAddress expanded {};
    TEST_ASSERT_TRUE(parseIpV6("2001:0db8:0:0:0:0:0:1", expanded));

    TEST_ASSERT_TRUE(compressed == expanded);

    IpAddress compressedWithHost {};
    TEST_ASSERT_TRUE(parseIpV6("2001:db8::2:1", compressedWithHost));

    IpAddress expandedWithHost {};
    TEST_ASSERT_TRUE(parseIpV6("2001:db8:0:0:0:0:2:1", expandedWithHost));

    TEST_ASSERT_TRUE(compressedWithHost == expandedWithHost);
    TEST_ASSERT_TRUE(compressed != compressedWithHost);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(parses_ipv4_addresses);
    RUN_TEST(parses_ipv6_addresses);
    RUN_TEST(compares_equivalent_ip_addresses);
    return UNITY_END();
}
