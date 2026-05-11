#include <unity.h>

#include "validation.h"

#include <cstring>

using namespace ip_inv;

static void validates_ipv4_addresses() {
    struct TestCase {
        const char* value;
        bool expected;
    };

    const TestCase tests[] = {
        {"192.168.1.1", true},
        {"127.0.0.1", true},
        {"0.0.0.0", true},
        {"255.255.255.255", true},
        {"256.1.1.1", false},
        {"192.168.1", false},
        {"192.168.1.1.1", false},
        {"not-an-ip", false},
        {"", false},
    };

    for (const TestCase& test : tests) {
        TEST_ASSERT_EQUAL(test.expected, isValidIpv4Address(test.value, strlen(test.value)));
    }
}

static void validates_ipv6_addresses() {
    struct TestCase {
        const char* value;
        bool expected;
    };

    const TestCase tests[] = {
        {"::1", true},
        {"2001:db8::1", true},
        {"fe80::1", true},
        {"2001:db8:85a3::8a2e:370:7334", true},
        {"2001:db8:::1", false},
        {"12345::1", false},
        {"not-an-ip", false},
        {"", false},
    };

    for (const TestCase& test : tests) {
        TEST_ASSERT_EQUAL(test.expected, isValidIpv6Address(test.value, strlen(test.value)));
    }
}

static void validates_ip_addresses_by_type() {
    struct TestCase {
        IpAddress address;
        bool expected;
    };

    const TestCase tests[] = {
        {{"192.168.1.1", IpType::IPv4}, true},
        {{"2001:db8::1", IpType::IPv6}, true},
        {{"2001:db8::1", IpType::IPv4}, false},
        {{"192.168.1.1", IpType::IPv6}, false},
        {{"bad-ip", IpType::IPv4}, false},
        {{"bad-ip", IpType::IPv6}, false},
    };

    for (const TestCase& test : tests) {
        TEST_ASSERT_EQUAL(test.expected, isValidIPAddress(test.address));
    }
}

static void validates_ports() {
    struct TestCase {
        i32 value;
        bool expected;
    };

    const TestCase tests[] = {
        {1, true},
        {80, true},
        {443, true},
        {65534, true},
        {0, false},
        {-1, false},
        {65535, false},
        {70000, false},
    };

    for (const TestCase& test : tests) {
        TEST_ASSERT_EQUAL(test.expected, isValidPort(test.value));
    }
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(validates_ipv4_addresses);
    RUN_TEST(validates_ipv6_addresses);
    RUN_TEST(validates_ip_addresses_by_type);
    RUN_TEST(validates_ports);
    return UNITY_END();
}
