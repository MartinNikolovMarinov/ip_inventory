#pragma once

#include "inventory/inventory_types.h"
#include "types.h"

#include <filesystem>
#include <string>
#include <vector>

struct sqlite3;

namespace ip_inv::test {

struct IpPoolRow {
    i32 ipType = 0;
    std::string displayIp;
};

std::string makeTestDatabaseName();
std::filesystem::path testDatabaseRoot();
std::filesystem::path testDatabasePath(const std::string& databaseName);

IpAddress makeAddress(const char* value);

sqlite3* openTestDatabase(const std::string& databaseName);
void execSql(sqlite3* db, const char* sql);
void assertTableExists(sqlite3* db, const char* tableName);
void clearTables(sqlite3* db);

std::vector<IpPoolRow> listIpPoolRows(const std::string& databaseName);
void prepareDatabaseForTest(const std::string& databaseName);
void assertIpPoolRows(const std::vector<IpPoolRow>& actual, const std::vector<IpPoolRow>& expected);

} // namespace ip_inv::test
