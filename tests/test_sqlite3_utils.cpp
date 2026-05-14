#include "test_sqlite3_utils.h"

#include <unity.h>

#include "inventory/sqllite3_repository.h"
#include "ip_utils.h"
#include "sqlite/sqlite.h"

#include <sqlite3.h>

#include <chrono>
#include <format>
#include <random>
#include <utility>

namespace ip_inv::test {

std::string makeTestDatabaseName() {
    const auto now = std::chrono::system_clock::now().time_since_epoch().count();

    std::random_device randomDevice;
    std::mt19937 rng(randomDevice());
    std::uniform_int_distribution<u32> distribution(100000, 999999);

    return std::format("sqlite3_repository_test_{}_{}.sqlite3", now, distribution(rng));
}

std::filesystem::path testDatabaseRoot() {
    return IP_INVENTORY_TEST_DATA_ROOT;
}

std::filesystem::path testDatabasePath(const std::string& databaseName) {
    return testDatabaseRoot() / databaseName;
}

IpAddress makeAddress(const char* value) {
    IpAddress address {};
    address.str = value;

    if (parseIpV4(address)) {
        address.type = IpType::IPv4;
    }
    else if (parseIpV6(address)) {
        address.type = IpType::IPv6;
    }
    else {
        TEST_FAIL_MESSAGE(value);
    }

    return address;
}

void execSql(sqlite3* db, const char* sql) {
    char* error = nullptr;
    const i32 result = sqlite3_exec(db, sql, nullptr, nullptr, &error);
    if (result != SQLITE_OK) {
        const std::string message = error == nullptr ? "unknown sqlite error" : error;
        sqlite3_free(error);
        TEST_FAIL_MESSAGE(message.c_str());
    }
}

sqlite3* openTestDatabase(const std::string& databaseName) {
    sqlite3* db = nullptr;
    const i32 result = sqlite3_open_v2(
        testDatabasePath(databaseName).string().c_str(),
        &db,
        SQLITE_OPEN_READWRITE,
        nullptr
    );

    if (result != SQLITE_OK) {
        const std::string message = db == nullptr ? "unknown sqlite error" : sqlite3_errmsg(db);
        if (db != nullptr) {
            sqlite3_close(db);
        }
        TEST_FAIL_MESSAGE(message.c_str());
    }

    return db;
}

void assertTableExists(sqlite3* db, const char* tableName) {
    SqliteStatement stmt(
        db,
        R"sql(
            SELECT EXISTS(
                SELECT 1
                FROM sqlite_master
                WHERE type = 'table' AND name = ?
            )
        )sql"
    );

    stmt.bindText(1, tableName);

    TEST_ASSERT_TRUE(stmt.stepRow());
    TEST_ASSERT_EQUAL_MESSAGE(1, stmt.columnInt(0), tableName);
}

void clearTables(sqlite3* db) {
    execSql(
        db,
        R"sql(
            DELETE FROM ip_pool;
            DELETE FROM reserved_ips;
            DELETE FROM services;
        )sql"
    );
}

std::vector<IpPoolRow> listIpPoolRows(const std::string& databaseName) {
    sqlite3* db = openTestDatabase(databaseName);

    SqliteStatement stmt(
        db,
        R"sql(
            SELECT ip_type, display_ip
            FROM ip_pool
            ORDER BY ip_type, display_ip
        )sql"
    );

    std::vector<IpPoolRow> rows;
    while (stmt.stepRow()) {
        IpPoolRow row;
        row.ipType = stmt.columnInt(0);
        row.displayIp = stmt.columnText(1);
        rows.push_back(std::move(row));
    }

    sqlite3_close(db);

    return rows;
}

void prepareDatabaseForTest(const std::string& databaseName) {
    IpInventoryRepositorySqlLite repository(databaseName);
    repository.initializeDb(true, IP_INVENTORY_SOURCE_DIR "/schema/001_init_db.sql");

    sqlite3* db = openTestDatabase(databaseName);

    assertTableExists(db, "services");
    assertTableExists(db, "reserved_ips");
    assertTableExists(db, "ip_pool");

    clearTables(db);
    sqlite3_close(db);
}

void assertIpPoolRows(const std::vector<IpPoolRow>& actual, const std::vector<IpPoolRow>& expected) {
    TEST_ASSERT_EQUAL(expected.size(), actual.size());

    for (usize i = 0; i < expected.size(); ++i) {
        TEST_ASSERT_EQUAL(expected[i].ipType, actual[i].ipType);
        TEST_ASSERT_EQUAL_STRING(expected[i].displayIp.c_str(), actual[i].displayIp.c_str());
    }
}

} // namespace ip_inv::test
