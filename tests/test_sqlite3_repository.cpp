#include <unity.h>

#include "inventory/sqllite3_repository.h"
#include "test_sqlite3_utils.h"

#include <filesystem>
#include <string>

using namespace ip_inv;

// TODO: Cover all edge cases for the sqlite repository class

namespace {

std::string g_databaseName;
std::filesystem::path g_databasePath;

void prepareRepositoryTestDatabase() {
    test::prepareDatabaseForTest(g_databaseName);
}

void addIpAddressesHandlesIpv4Ipv6DuplicatesAndMixedExistingRows() {
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

void clearExpiredReservationsRemovesPastReservation() {
    IpInventoryRepositorySqlLite repository(g_databaseName);
    repository.initializeDb(false);

    constexpr const char* SERVICE_ID = "service-a";
    constexpr const char* IPV4_STR = "95.44.73.19";
    const IpAddress ipv4 = test::makeAddress(IPV4_STR);

    TEST_ASSERT_TRUE(repository.addIpAddresses({ipv4}).success());

    ReserveIpResult reserveResult = repository.reserveIpAddress(SERVICE_ID, IpTypeSelection::IPv4, 1);
    TEST_ASSERT_TRUE(reserveResult.success());
    TEST_ASSERT_EQUAL(1, reserveResult.reservedIps.size());
    TEST_ASSERT_TRUE(ipv4 == reserveResult.reservedIps[0]);

    ReservedIpsResult reservedBeforeGc = repository.getReservedIps();
    TEST_ASSERT_TRUE(reservedBeforeGc.success());
    TEST_ASSERT_EQUAL(1, reservedBeforeGc.reservedIps.size());
    TEST_ASSERT_EQUAL_STRING(SERVICE_ID, reservedBeforeGc.reservedIps[0].serviceId.c_str());
    TEST_ASSERT_TRUE(ipv4 == reservedBeforeGc.reservedIps[0].address);

    repository.clearExpiredReservations();

    ReservedIpsResult reservedAfterGc = repository.getReservedIps();
    TEST_ASSERT_TRUE(reservedAfterGc.success());
    TEST_ASSERT_EQUAL(0, reservedAfterGc.reservedIps.size());
}

void reserveIpReturnsExistingReservationForService() {
    IpInventoryRepositorySqlLite repository(g_databaseName);
    repository.initializeDb(false);

    constexpr const char* SERVICE_ID = "service-a";
    const IpAddress ipv4A = test::makeAddress("95.44.73.19");
    const IpAddress ipv4B = test::makeAddress("95.44.73.18");

    TEST_ASSERT_TRUE(repository.addIpAddresses({ipv4A, ipv4B}).success());

    ReserveIpResult firstReserve = repository.reserveIpAddress(SERVICE_ID, IpTypeSelection::IPv4, 60);
    TEST_ASSERT_TRUE(firstReserve.success());
    TEST_ASSERT_EQUAL(1, firstReserve.reservedIps.size());

    ReserveIpResult secondReserve = repository.reserveIpAddress(SERVICE_ID, IpTypeSelection::IPv4, 60);
    TEST_ASSERT_TRUE(secondReserve.success());
    TEST_ASSERT_EQUAL(1, secondReserve.reservedIps.size());
    TEST_ASSERT_TRUE(firstReserve.reservedIps[0] == secondReserve.reservedIps[0]);

    ReservedIpsResult reservedIps = repository.getReservedIps();
    TEST_ASSERT_TRUE(reservedIps.success());
    TEST_ASSERT_EQUAL(1, reservedIps.reservedIps.size());
    TEST_ASSERT_EQUAL_STRING(SERVICE_ID, reservedIps.reservedIps[0].serviceId.c_str());
    TEST_ASSERT_TRUE(firstReserve.reservedIps[0] == reservedIps.reservedIps[0].address);
}

} // namespace

void setUp(void) {
    prepareRepositoryTestDatabase();
}

void tearDown(void) {}

i32 main() {
    std::filesystem::create_directories(test::testDatabaseRoot());

    g_databaseName = test::makeTestDatabaseName();
    g_databasePath = test::testDatabasePath(g_databaseName);

    UNITY_BEGIN();
    RUN_TEST(addIpAddressesHandlesIpv4Ipv6DuplicatesAndMixedExistingRows);
    RUN_TEST(clearExpiredReservationsRemovesPastReservation);
    RUN_TEST(reserveIpReturnsExistingReservationForService);
    const i32 result = UNITY_END();

    std::filesystem::remove(g_databasePath);

    return result;
}
