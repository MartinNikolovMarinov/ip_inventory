#include "inventory/sqllite3_repository.h"
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

    SqliteTransaction tx(m_db);

    //
    // Ensure service exists
    //
    SqliteStatement insertServiceStm(
        m_db,
        R"sql(
            INSERT INTO services (service_id)
            VALUES (?)
            ON CONFLICT(service_id) DO NOTHING;
        )sql"
    );

    insertServiceStm.bindText(1, serviceId);
    insertServiceStm.execute();

    //
    // Resolve internal service id
    //
    SqliteStatement findServiceStm(
        m_db,
        R"sql(
            SELECT id
            FROM services
            WHERE service_id = ?;
        )sql"
    );

    findServiceStm.bindText(1, serviceId);

    if (!findServiceStm.stepRow()) {
        ret.status.error = InventoryError::DbError;
        ret.status.detail = "Failed to resolve service";
        return ret;
    }

    const i64 serviceDbId = findServiceStm.columnInt64(0);

    //
    // Create reservation row
    //
    SqliteStatement insertReservationStm(
        m_db,
        R"sql(
            INSERT INTO reserved_ips (
                service_id,
                expiration_time
            )
            VALUES (?, ?);
        )sql"
    );

    insertReservationStm.bindInt64(1, serviceDbId);
    insertReservationStm.bindInt64(2, expirationTime);

    insertReservationStm.execute();

    const i64 reservationId = sqlite3_last_insert_rowid(m_db);

    //
    // Reserve first available IP matching requested type
    //
    SqliteStatement reserveIpStm(
        m_db,
        R"sql(
            UPDATE ip_pool
            SET reserved_ip = ?
            WHERE rowid = (
                SELECT rowid
                FROM ip_pool
                WHERE (? = 0 OR ip_type = ?)
                  AND assigned_id IS NULL
                  AND reserved_ip IS NULL
                LIMIT 1
            );
        )sql"
    );

    i32 ipType = 0;
    switch (ipTypeSelection) {
        case IpTypeSelection::IPv4:
            ipType = 4;
            break;
        case IpTypeSelection::IPv6:
            ipType = 6;
            break;
        case IpTypeSelection::Both:
            ipType = 0;
            break;
    }

    reserveIpStm.bindInt64(1, reservationId);
    reserveIpStm.bindInt(2, ipType);
    reserveIpStm.bindInt(3, ipType);

    reserveIpStm.execute();

    if (sqlite3_changes(m_db) != 1) {
        ret.status.error = InventoryError::IpUnavailable;
        ret.status.detail = "No available IP addresses";

        return ret;
    }

    //
    // Read back reserved IP
    //
    SqliteStatement fetchReservedIpStm(
        m_db,
        R"sql(
            SELECT
                ip_type,
                ip_bytes,
                display_ip
            FROM ip_pool
            WHERE reserved_ip = ?;
        )sql"
    );

    fetchReservedIpStm.bindInt64(1, reservationId);

    if (!fetchReservedIpStm.stepRow()) {
        ret.status.error = InventoryError::DbError;
        ret.status.detail = "Failed to fetch reserved IP";
        return ret;
    }

    IpAddress reservedIp {};
    reservedIp.type =
        fetchReservedIpStm.columnInt(0) == 4
            ? IpType::IPv4
            : IpType::IPv6;

    const void* blob = fetchReservedIpStm.columnBlob(1);
    const i32 blobSize = fetchReservedIpStm.columnBytes(1);

    std::memcpy(reservedIp.bytes, blob, blobSize);

    reservedIp.str = fetchReservedIpStm.columnText(2);

    ret.reservedIps.push_back(std::move(reservedIp));

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

} // namespace

} // namespace ip_inv
