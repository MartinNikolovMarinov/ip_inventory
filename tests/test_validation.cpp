#include <unity.h>

#include "inventory/inventory_types.h"
#include "validation.h"

using namespace ip_inv;

void setUp() {}

void tearDown() {}

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

static void validates_database_names() {
    struct TestCase {
        const char* value;
        bool expected;
    };

    const TestCase tests[] = {
        {"ip_inventory.sqlite3", true},
        {"db-name_1.sqlite3", true},
        {"a", true},
        {"", false},
        {".", false},
        {"..", false},
        {"bad/name.sqlite3", false},
        {"bad name.sqlite3", false},
        {"bad:name.sqlite3", false},
    };

    for (const TestCase& test : tests) {
        TEST_ASSERT_EQUAL_MESSAGE(test.expected, isValidDatabaseName(test.value), test.value);
    }
}

static void validates_service_ids() {
    const std::string longServiceId(129, 'a');

    struct TestCase {
        std::string value;
        bool expected;
    };

    const TestCase tests[] = {
        {"service-a", true},
        {"service_a.1", true},
        {"with spaces", true},
        {"", false},
        {longServiceId, false},
        {"bad\nservice", false},
        {"bad\tservice", false},
        {std::string("bad") + char(127), false},
    };

    for (const TestCase& test : tests) {
        TEST_ASSERT_EQUAL_MESSAGE(test.expected, isValidServiceId(test.value), test.value.c_str());
    }
}

static void validates_ip_type_selections() {
    TEST_ASSERT_TRUE(isValidIpTypeSelection(IpTypeSelection::IPv4));
    TEST_ASSERT_TRUE(isValidIpTypeSelection(IpTypeSelection::IPv6));
    TEST_ASSERT_TRUE(isValidIpTypeSelection(IpTypeSelection::Both));
    TEST_ASSERT_FALSE(isValidIpTypeSelection(IpTypeSelection::Undefined));
}

static void validates_ip_addresses() {
    const IpAddress ipv4 {"95.44.73.19", {95, 44, 73, 19}, IpType::IPv4};
    const IpAddress ipv6 {"2a01:5a9:1a4:95c::1", {}, IpType::IPv6};
    const IpAddress emptyText {"", {95, 44, 73, 19}, IpType::IPv4};
    const IpAddress undefined {"95.44.73.19", {95, 44, 73, 19}, IpType::Undefined};

    TEST_ASSERT_TRUE(isValidIpAddress(ipv4));
    TEST_ASSERT_TRUE(isValidIpAddress(ipv6));
    TEST_ASSERT_FALSE(isValidIpAddress(emptyText));
    TEST_ASSERT_FALSE(isValidIpAddress(undefined));
}

static void validates_ip_address_lists() {
    const IpAddress ipv4 {"95.44.73.19", {95, 44, 73, 19}, IpType::IPv4};
    const IpAddress ipv6 {"2a01:5a9:1a4:95c::1", {}, IpType::IPv6};
    const IpAddress invalid {"", {}, IpType::IPv4};

    TEST_ASSERT_TRUE(isValidIpAddressList({ipv4}));
    TEST_ASSERT_TRUE(isValidIpAddressList({ipv4, ipv6}));
    TEST_ASSERT_FALSE(isValidIpAddressList({}));
    TEST_ASSERT_FALSE(isValidIpAddressList({ipv4, invalid}));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(validates_ports);
    RUN_TEST(validates_database_names);
    RUN_TEST(validates_service_ids);
    RUN_TEST(validates_ip_type_selections);
    RUN_TEST(validates_ip_addresses);
    RUN_TEST(validates_ip_address_lists);
    return UNITY_END();
}
