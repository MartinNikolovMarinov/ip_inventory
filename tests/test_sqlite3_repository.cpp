#include <unity.h>

#include "inventory/sqllite3_repository.h"
#include "test_sqlite3_utils.h"

#include <filesystem>
#include <string>

using namespace ip_inv;

namespace {

std::string g_databaseName;
std::filesystem::path g_databasePath;

} // namespace

void setUp() {
    test::prepareDatabaseForTest(g_databaseName);
}

void tearDown() {}

static void add_ip_addresses_handles_ipv4_ipv6_duplicates_and_mixed_existing_rows() {
    IpInventoryRepositorySqlLite repository(g_databaseName);
    repository.initializeDb(false);

    constexpr const char* IPV4_A_STR = "95.44.73.19";
    constexpr const char* IPV4_B_STR = "95.44.73.18";
    constexpr const char* IPV4_C_STR = "95.44.73.17";
    constexpr const char* IPV6_A_STR = "2a01:5a9:1a4:95c::1";
    constexpr const char* IPV6_B_STR = "2a01:5a9:1a4:95c::2";

    const IpAddress ipv4A = test::makeAddress(IPV4_A_STR);
    const IpAddress ipv4B = test::makeAddress(IPV4_B_STR);
    const IpAddress ipv4C = test::makeAddress(IPV4_C_STR);
    const IpAddress ipv6A = test::makeAddress(IPV6_A_STR);
    const IpAddress ipv6B = test::makeAddress(IPV6_B_STR);

    TEST_ASSERT_TRUE(repository.addIpAddresses({ipv4A}).success());
    test::assertIpPoolRows(test::listIpPoolRows(g_databaseName), {
        {4, IPV4_A_STR},
    });

    TEST_ASSERT_TRUE(repository.addIpAddresses({ipv6A}).success());
    test::assertIpPoolRows(test::listIpPoolRows(g_databaseName), {
        {4, IPV4_A_STR},
        {6, IPV6_A_STR},
    });

    TEST_ASSERT_TRUE(repository.addIpAddresses({ipv4B, ipv6B}).success());
    test::assertIpPoolRows(test::listIpPoolRows(g_databaseName), {
        {4, IPV4_B_STR},
        {4, IPV4_A_STR},
        {6, IPV6_A_STR},
        {6, IPV6_B_STR},
    });

    TEST_ASSERT_TRUE(repository.addIpAddresses({ipv4A, ipv4B, ipv6A, ipv6B}).success());
    test::assertIpPoolRows(test::listIpPoolRows(g_databaseName), {
        {4, IPV4_B_STR},
        {4, IPV4_A_STR},
        {6, IPV6_A_STR},
        {6, IPV6_B_STR},
    });

    TEST_ASSERT_TRUE(repository.addIpAddresses({ipv4A, ipv4C}).success());
    test::assertIpPoolRows(test::listIpPoolRows(g_databaseName), {
        {4, IPV4_C_STR},
        {4, IPV4_B_STR},
        {4, IPV4_A_STR},
        {6, IPV6_A_STR},
        {6, IPV6_B_STR},
    });
}

i32 main() {
    std::filesystem::create_directories(test::testDatabaseRoot());

    g_databaseName = test::makeTestDatabaseName();
    g_databasePath = test::testDatabasePath(g_databaseName);

    UNITY_BEGIN();
    RUN_TEST(add_ip_addresses_handles_ipv4_ipv6_duplicates_and_mixed_existing_rows);
    const i32 result = UNITY_END();

    std::filesystem::remove(g_databasePath);

    return result;
}
