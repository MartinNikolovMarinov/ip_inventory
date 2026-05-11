#include "inventory/sqllite3_repository.h"

#include "../sql/sqlite_queries.h"

#include <sqlite3.h>

#include <filesystem>
#include <format>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

namespace ip_inv {

namespace {

std::filesystem::path sqliteDatabaseRoot() {
#if defined(IP_INVENTORY_SQLITE3_DB_ROOT)
    return IP_INVENTORY_SQLITE3_DB_ROOT;
#else
    return std::filesystem::path(".") / "db";
#endif
}

std::filesystem::path sqliteDatabasePath(const std::string& databaseName) {
    return sqliteDatabaseRoot() / databaseName;
}

void assertSqliteOk(int result, sqlite3* db, const char* operation) {
    if (result == SQLITE_OK) {
        return;
    }

    const char* error = db == nullptr ? "unknown sqlite error" : sqlite3_errmsg(db);
    throw std::runtime_error(std::format("{}: {}", operation, error));
}

} // namespace

IpInventoryRepositorySqlLite::IpInventoryRepositorySqlLite(std::string databaseName)
    : m_databaseName(std::move(databaseName)),
      m_databasePath(sqliteDatabasePath(m_databaseName)) {}

IpInventoryRepositorySqlLite::~IpInventoryRepositorySqlLite() noexcept {
    std::lock_guard lock(m_dbMutex);
    if (m_db != nullptr) {
        sqlite3_close(m_db);
        m_db = nullptr;
    }
}

void IpInventoryRepositorySqlLite::initializeDb() {
    std::cout << "Initializing SQL LITE database" << std::endl;

    std::lock_guard lock(m_dbMutex);

    if (m_db != nullptr) {
        return;
    }

    std::filesystem::create_directories(m_databasePath.parent_path());

    sqlite3* db = nullptr;
    i32 openResult = sqlite3_open_v2(
        m_databasePath.string().c_str(),
        &db,
        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX,
        nullptr
    );
    if (openResult != SQLITE_OK) {
        const char* error = db == nullptr ? "unknown sqlite error" : sqlite3_errmsg(db);
        if (db != nullptr) {
            sqlite3_close(db);
        }
        throw std::runtime_error(std::format("open database: {}", error));
    }

    m_db = db;
    assertSqliteOk(sqlite3_exec(m_db, sqliteEnableForeignKeys, nullptr, nullptr, nullptr), m_db, "enable foreign keys");
    assertSqliteOk(sqlite3_exec(m_db, sqliteCreateIpPoolTable, nullptr, nullptr, nullptr), m_db, "create ip_pool");

    std::cout << "SQL LITE database intialized successfully" << std::endl;
}

AddToPoolResult IpInventoryRepositorySqlLite::addIpAddresses(const std::vector<IpAddress>&) {
    AddToPoolResult ret;
    // TODO: implement..
    return ret;
}

} // namespace ip_inv
