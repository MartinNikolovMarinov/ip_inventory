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

AddToPoolResult IpInventoryRepositorySqlLite::addIpAddresses(const std::vector<IpAddress>& addresses) {
    AddToPoolResult ret;
    std::lock_guard lock(m_dbMutex);

    if (m_db == nullptr) {
        ret.status.error = InventoryError::InvalidIp;
        ret.status.detail = "Failed to add ip address; reason: database is not initialized";
        return ret;
    }

    sqlite3_stmt* statement = nullptr;
    assertSqliteOk(sqlite3_prepare_v2(m_db, sqliteInsertIpPoolAddress, -1, &statement, nullptr), m_db, "prepare insert ip_pool");

    for (const IpAddress& address : addresses) {
        const char* ipType = address.type == IpType::IPv4 ? "IPv4" : "IPv6";
        const int byteCount = static_cast<int>(IpAddress::byteCount(address.type));

        assertSqliteOk(sqlite3_bind_text(statement, 1, ipType, -1, SQLITE_TRANSIENT), m_db, "bind ip_type");
        assertSqliteOk(sqlite3_bind_blob(statement, 2, address.bytes, byteCount, SQLITE_TRANSIENT), m_db, "bind ip_bytes");
        assertSqliteOk(sqlite3_bind_text(statement, 3, address.value.c_str(), -1, SQLITE_TRANSIENT), m_db, "bind display_ip");

        const int stepResult = sqlite3_step(statement);
        if (stepResult != SQLITE_DONE) {
            sqlite3_finalize(statement);
            assertSqliteOk(stepResult, m_db, "insert ip_pool");
        }

        assertSqliteOk(sqlite3_reset(statement), m_db, "reset insert ip_pool");
        assertSqliteOk(sqlite3_clear_bindings(statement), m_db, "clear insert ip_pool bindings");
    }

    assertSqliteOk(sqlite3_finalize(statement), m_db, "finalize insert ip_pool");
    return ret;
}

} // namespace ip_inv
