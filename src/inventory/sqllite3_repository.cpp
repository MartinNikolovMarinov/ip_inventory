#include "inventory/sqllite3_repository.h"

#include <sqlite3.h>

#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

namespace ip_inv {

namespace {

std::filesystem::path sqliteDatabaseRoot();
std::filesystem::path sqliteDatabasePath(const std::string& databaseName);

std::string readTextFile(const std::filesystem::path& path);

void assertSqliteOk(int result, sqlite3* db, const char* operation);
void execSqlScriptFromFile(sqlite3* db, const std::filesystem::path& path);

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

void IpInventoryRepositorySqlLite::initializeDb(bool dropCreate, std::filesystem::path schemaInitScriptPath) {
    std::cout << "Initializing SQL LITE database" << std::endl;

    std::lock_guard lock(m_dbMutex);

    if (m_db != nullptr) {
        return;
    }

    std::filesystem::create_directories(m_databasePath.parent_path());

    // Connect to database
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

    if (dropCreate) {
        execSqlScriptFromFile(m_db, schemaInitScriptPath);
    }

    std::cout << "SQL LITE database intialized successfully" << std::endl;
}

AddToPoolResult IpInventoryRepositorySqlLite::addIpAddresses(const std::vector<IpAddress>& addresses) {
    AddToPoolResult ret;
    std::lock_guard lock(m_dbMutex);

    // TODO: implement
    return ret;
}

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

std::string readTextFile(const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error(std::format("open file '{}'", path.string()));
    }

    std::ostringstream content;
    content << file.rdbuf();
    return content.str();
}

void assertSqliteOk(int result, sqlite3* db, const char* operation) {
    if (result == SQLITE_OK) {
        return;
    }

    const char* error = db == nullptr ? "unknown sqlite error" : sqlite3_errmsg(db);
    throw std::runtime_error(std::format("{}: {}", operation, error));
}

// This function is very unsafe, but it is used only on db initialization by trusted code.
void execSqlScriptFromFile(sqlite3* db, const std::filesystem::path& path) {
    const std::string script = readTextFile(path);

    char* error = nullptr;
    const int result = sqlite3_exec(db, script.c_str(), nullptr, nullptr, &error);
    if (result != SQLITE_OK) {
        const std::string message = error == nullptr ? "unknown sqlite error" : error;
        sqlite3_free(error);
        throw std::runtime_error(std::format("execute SQL script '{}': {}", path.string(), message));
    }
}

} // namespace

} // namespace ip_inv
