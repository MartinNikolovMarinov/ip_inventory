#include "inventory/sqllite3_repository.h"
#include "inventory/inventory_types.h"
#include "sqlite/sqlite.h"

#include <sqlite3.h>

#include <cstring>
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

void assertSqliteOk(i32 result, sqlite3* db, const char* operation);
void execSqlScriptFromFile(sqlite3* db, const std::filesystem::path& path);

[[nodiscard]] bool findAvailableIpAddressQuery(sqlite3* db, IpType type, IpAddress& out);
void reserveIpAddressQuery(sqlite3* db, i64 serviceDbId, i64 expirationTime, const IpAddress& address);
void createServiceQuery(sqlite3* db, const std::string& serviceId);
i64 findServicePrimaryKeyQuery(sqlite3* db, const std::string& serviceId);

} // namespace

//======================================================================================================================
// PUBLIC
//======================================================================================================================

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
    else {
        // TODO: there should be a check to validate weather the necessary tables exist, or some migration scripts might be ran as well.
    }

    std::cout << "SQL LITE database intialized successfully" << std::endl;
}

AddToPoolResult IpInventoryRepositorySqlLite::addIpAddresses(const std::vector<IpAddress>& addresses) {
    AddToPoolResult ret;

    std::lock_guard lock(m_dbMutex);

    if (m_db == nullptr) {
        ret.status.error = InventoryError::DbNotInitialized;
        ret.status.detail = "Failed to add ip address; reason: database is not initialized";
        return ret;
    }

    std::cout << "Add IP addresses transaction started" << std::endl;

    // NOTE:
    // This executes N INSERT statements inside a single transaction using a reused prepared statement. SQLite prepared
    // statements require a fixed parameter count, so dynamically batching arbitrary row counts would require generating
    // SQL strings at runtime. To preserve prepared-statement safety and avoid manual SQL string construction /
    // injection risks, the insert is performed row-by-row within the transaction instead.
    SqliteTransaction tx(m_db);
    SqliteStatement insertStm(
        m_db,
        R"sql(
            INSERT INTO ip_pool (ip_type, ip_bytes, display_ip, assigned_id, reserved_ip)
            VALUES (?, ?, ?, NULL, NULL)
            ON CONFLICT(ip_type, ip_bytes) DO NOTHING;
        )sql"
    );

    for (const IpAddress& address : addresses) {
        const i32 ipType = address.type == IpType::IPv4 ? 4 : 6;
        const i32 byteCount = i32(IpAddress::byteCount(address.type));

        insertStm.bindInt(1, ipType);
        insertStm.bindBlob(2, address.bytes, byteCount);
        insertStm.bindText(3, address.str);

        insertStm.execute();

        insertStm.reset();
        insertStm.clearBindings();
    }

    tx.commit();

    std::cout << "Add IP addresses transaction successfull" << std::endl;

    return ret;
}

ReserveIpResult IpInventoryRepositorySqlLite::reserveIpAddress(
    const std::string& serviceId,
    IpTypeSelection ipTypeSelection,
    i64 expirationTime
) {
    ReserveIpResult ret;

    std::lock_guard lock(m_dbMutex);

    if (m_db == nullptr) {
        ret.status.error = InventoryError::DbNotInitialized;
        ret.status.detail = "Failed to reserve IP address; reason: database is not initialized";
        return ret;
    }

    bool ipv4IsRequested = ipTypeSelection == IpTypeSelection::IPv4 || ipTypeSelection == IpTypeSelection::Both;
    bool ipv6IsRequested = ipTypeSelection == IpTypeSelection::IPv6 || ipTypeSelection == IpTypeSelection::Both;

    IpAddress ipv4;
    if (ipv4IsRequested) {
        if (!findAvailableIpAddressQuery(m_db, IpType::IPv4, ipv4)) {
            ret.status.error = InventoryError::IpUnavailable;
            ret.status.detail = "Failed to reserve IP address; reason: no available IPv4 addresses";
            return ret;
        }
    }

    IpAddress ipv6;
    if (ipv6IsRequested) {
        if (!findAvailableIpAddressQuery(m_db, IpType::IPv6, ipv6)) {
            ret.status.error = InventoryError::IpUnavailable;
            ret.status.detail = "Failed to reserve IP address; reason: no available IPv6 addresses";
            return ret;
        }
    }

    SqliteTransaction tx(m_db);

    createServiceQuery(m_db, serviceId);
    i64 serviceDbId = findServicePrimaryKeyQuery(m_db, serviceId);

    if (ipv4IsRequested) {
        reserveIpAddressQuery(m_db, serviceDbId, expirationTime, ipv4);
        ret.reservedIps.push_back(std::move(ipv4));
    }
    if (ipv6IsRequested) {
        reserveIpAddressQuery(m_db, serviceDbId, expirationTime, ipv6);
        ret.reservedIps.push_back(std::move(ipv6));
    }

    tx.commit();

    return ret;
}

//======================================================================================================================
// Internal helper functions
//======================================================================================================================

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

void assertSqliteOk(i32 result, sqlite3* db, const char* operation) {
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
    const i32 result = sqlite3_exec(db, script.c_str(), nullptr, nullptr, &error);
    if (result != SQLITE_OK) {
        const std::string message = error == nullptr ? "unknown sqlite error" : error;
        sqlite3_free(error);
        throw std::runtime_error(std::format("execute SQL script '{}': {}", path.string(), message));
    }
}

void reserveIpAddressQuery(sqlite3* db, i64 serviceDbId, i64 expirationTime, const IpAddress& address) {
    SqliteStatement insertReservationStm(
        db,
        R"sql(
            INSERT INTO reserved_ips (service_id, expiration_time)
            VALUES (?, ?);
        )sql"
    );

    insertReservationStm.bindInt64(1, serviceDbId);
    insertReservationStm.bindInt64(2, expirationTime);
    insertReservationStm.execute();

    const i64 reservedIpId = sqlite3_last_insert_rowid(db);

    SqliteStatement updatePoolStm(
        db,
        R"sql(
            UPDATE ip_pool
            SET reserved_ip = ?
            WHERE ip_type = ?
                AND ip_bytes = ?
                AND assigned_id IS NULL
                AND reserved_ip IS NULL;
        )sql"
    );

    updatePoolStm.bindInt64(1, reservedIpId);
    updatePoolStm.bindInt(2, i32(address.type));
    updatePoolStm.bindBlob(3, address.bytes, i32(IpAddress::byteCount(address.type)));
    updatePoolStm.execute();

    if (sqlite3_changes(db) != 1) {
        throw std::runtime_error("failed to reserve IP address");
    }
}

void createServiceQuery(sqlite3* db, const std::string& serviceId) {
    SqliteStatement insertServiceStm(
        db,
        R"sql(
            INSERT INTO services (service_id)
            VALUES (?)
            ON CONFLICT(service_id) DO NOTHING;
        )sql"
    );

    insertServiceStm.bindText(1, serviceId);
    insertServiceStm.execute();
}

i64 findServicePrimaryKeyQuery(sqlite3* db, const std::string& serviceId) {
    SqliteStatement findServiceStm(
        db,
        R"sql(
            SELECT id
            FROM services
            WHERE service_id = ?;
        )sql"
    );

    findServiceStm.bindText(1, serviceId);

    if (!findServiceStm.stepRow()) {
        throw std::runtime_error("failed to resolve service primary key");
    }

    i64 ret = findServiceStm.columnInt64(0);
    return ret;
}

bool findAvailableIpAddressQuery(sqlite3* db, IpType type, IpAddress& out) {
    SqliteStatement findServiceStm(
        db,
        R"sql(
            SELECT ip_bytes, display_ip
            FROM ip_pool
            WHERE ip_type = ?
                AND assigned_id IS NULL
                AND reserved_ip IS NULL
            LIMIT 1;
        )sql"
    );

    findServiceStm.bindInt(1, i32(type));

    if (!findServiceStm.stepRow()) {
        // No available IP
        return false;
    }

    const void* blob = findServiceStm.columnBlob(0);
    int blobSize = findServiceStm.columnBytes(0);

    std::memcpy(out.bytes, blob, blobSize);
    out.str = findServiceStm.columnText(1);
    out.type = type;

    return true;
}

} // namespace

} // namespace ip_inv
