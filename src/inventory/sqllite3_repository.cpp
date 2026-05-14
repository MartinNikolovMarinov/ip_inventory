#include "inventory/sqllite3_repository.h"
#include "inventory/inventory_types.h"
#include "sqlite/sqlite.h"

#include <sqlite3.h>

#include <chrono>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <utility>

namespace ip_inv {

namespace {

struct Reservation {
    i64 reservedId;
    i64 serviceDbId;
};

std::filesystem::path sqliteDatabaseRoot();
std::filesystem::path sqliteDatabasePath(const std::string& databaseName);

std::string readTextFile(const std::filesystem::path& path);

void execSqlScriptFromFile(sqlite3* db, const std::filesystem::path& path);

[[nodiscard]] bool isIpAddressAvailableQuery(sqlite3* db, const IpAddress& address);
[[nodiscard]] bool findAvailableIpAddressQuery(sqlite3* db, IpType type, IpAddress& out);
[[nodiscard]] bool findIpAddressReservationQuery(sqlite3* db, const IpAddress& address, Reservation& out);
void reserveIpAddressQuery(sqlite3* db, i64 serviceDbId, i64 expirationTime, const IpAddress& address);
void assignIpAddressQuery(sqlite3* db, const IpAddress& address, i64 serviceDbId, i64 reservationId);

void createServiceQuery(sqlite3* db, const std::string& serviceId);
[[nodiscard]] i64 findServiceDbIdQuery(sqlite3* db, const std::string& serviceId);


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

    assertSqliteOk(
        sqlite3_exec(m_db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr),
        m_db,
        "Foreign keys much be enabled for every connection"
    );

    std::cout << "SQL LITE database intialized successfully" << std::endl;
}

InventoryStatus IpInventoryRepositorySqlLite::addIpAddresses(const std::vector<IpAddress>& addresses) {
    std::lock_guard lock(m_dbMutex);

    if (m_db == nullptr) {
        InventoryStatus status;
        status.error = InventoryError::DbNotInitialized;
        status.detail = "Failed to add ip address; reason: database is not initialized";
        return status;
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
            INSERT INTO ip_pool (ip_type, ip_bytes, display_ip, assigned_id, reserved_id)
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

    const i32 updated = sqlite3_changes(m_db);

    tx.commit();

    std::cout << "Added " << updated << " ip addresses" << std::endl;

    return InventoryStatus::OkStatus();
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

    SqliteTransaction tx(m_db);

    IpAddress ipv4;
    if (ipv4IsRequested) {
        std::cout << "Requested to reserve IPV4 address" << std::endl;

        if (!findAvailableIpAddressQuery(m_db, IpType::IPv4, ipv4)) {
            ret.status.error = InventoryError::IpUnavailable;
            ret.status.detail = "Failed to reserve IP address; reason: no available IPv4 addresses";
            return ret;
        }
    }

    IpAddress ipv6;
    if (ipv6IsRequested) {
        std::cout << "Requested to reserve IPV6 address" << std::endl;

        if (!findAvailableIpAddressQuery(m_db, IpType::IPv6, ipv6)) {
            ret.status.error = InventoryError::IpUnavailable;
            ret.status.detail = "Failed to reserve IP address; reason: no available IPv6 addresses";
            return ret;
        }
    }

    createServiceQuery(m_db, serviceId);
    i64 serviceDbId = findServiceDbIdQuery(m_db, serviceId);
    if (serviceDbId < 0) {
        throw std::runtime_error("Failed to read service that should have been created just now");
    }

    if (ipv4IsRequested) {
        reserveIpAddressQuery(m_db, serviceDbId, expirationTime, ipv4);
        ret.reservedIps.push_back(std::move(ipv4));
    }
    if (ipv6IsRequested) {
        reserveIpAddressQuery(m_db, serviceDbId, expirationTime, ipv6);
        ret.reservedIps.push_back(std::move(ipv6));
    }

    const i32 updated = sqlite3_changes(m_db);

    tx.commit();

    std::cout << "Reserved " << updated << " ip addresses" << std::endl;

    return ret;
}

InventoryStatus IpInventoryRepositorySqlLite::assignIpAddress(
    const std::string& serviceId,
    std::vector<IpAddress>&& addresses
) {
    std::lock_guard lock(m_dbMutex);

    if (m_db == nullptr) {
        InventoryStatus status;
        status.error = InventoryError::DbNotInitialized;
        status.detail = "Failed to assign IP address; reason: database is not initialized";
        return status;
    }

    SqliteTransaction tx (m_db);

    i64 serviceDbId = findServiceDbIdQuery(m_db, serviceId);
    if (serviceDbId < 0) {
        InventoryStatus status;
        status.error = InventoryError::ServiceNotFound;
        status.detail = "Failed to assign IP address; reason: service not found";
        return status;
    }

    for (const auto& address : addresses) {
        Reservation reservation;

        if (!isIpAddressAvailableQuery(m_db, addresses[0])) {
            InventoryStatus status;
            status.error = InventoryError::IpNotAvailable;
            status.detail = std::format(
                "Failed to assign IP address; reason: ip address {} is not available",
                addresses[0].str
            );
            return status;
        }

        if (!findIpAddressReservationQuery(m_db, addresses[0], reservation)) {
            InventoryStatus status;
            status.error = InventoryError::IpNotReserved;
            status.detail = std::format(
                "Failed to assign IP address; reason: ip address {} not reserved",
                addresses[0].str
            );
            return status;
        }

        if (reservation.serviceDbId != serviceDbId) {
            InventoryStatus status;
            status.error = InventoryError::IpReservedForDifferentService;
            status.detail = std::format(
                "Failed to assign IP address; reason: ip address {} reserved for a differnet service",
                addresses[0].str
            );
            return status;
        }

        assignIpAddressQuery(m_db, addresses[0], serviceDbId, reservation.reservedId);
        std::cout << "Assigned IP " << addresses[0].str << " for service " << serviceId;
    }

    tx.commit();

    return InventoryStatus::OkStatus();
}

InventoryStatus IpInventoryRepositorySqlLite::terminateIpAssignment(const std::string& serviceId, std::vector<IpAddress>&& addresses) {
    std::lock_guard lock(m_dbMutex);

    if (m_db == nullptr) {
        InventoryStatus status;
        status.error = InventoryError::DbNotInitialized;
        status.detail = "Failed to terminate IP address; reason: database is not initialized";
        return status;
    }

    // TODO: implement terminate.
    (void)serviceId;
    (void)addresses;

    return InventoryStatus::OkStatus();
}

InventoryStatus IpInventoryRepositorySqlLite::changeServiceId(
    const std::string& serviceIdOld,
    const std::string& serviceIdNew
) {
    std::lock_guard lock(m_dbMutex);

    if (m_db == nullptr) {
        InventoryStatus status;
        status.error = InventoryError::DbNotInitialized;
        status.detail = "Failed to change service id; reason: database is not initialized";
        return status;
    }

    SqliteTransaction tx (m_db);

    if (findServiceDbIdQuery(m_db, serviceIdOld) < 0) {
        InventoryStatus status;
        status.error = InventoryError::ServiceNotFound;
        status.detail = std::format(
            "Failed to change service id; reason: service ({}) does not exist",
            serviceIdOld
        );
        return status;
    }

    SqliteStatement changeServiceIdStm(
        m_db,
        R"sql(
            UPDATE services
            SET service_id = ?
            WHERE service_id = ?
        )sql"
    );

    changeServiceIdStm.bindText(1, serviceIdNew);
    changeServiceIdStm.bindText(2, serviceIdOld);
    changeServiceIdStm.execute();

    const i32 updated = sqlite3_changes(m_db);
    if (updated != 1) {
        throw std::runtime_error(
            std::format("service change query must update exactly one record; actual = {}", updated)
        );
    }

    tx.commit();

    std::cout << "Service id changed from " << serviceIdOld << " to " << serviceIdNew << std::endl;

    return InventoryStatus::OkStatus();
}

ServiceIpsResult IpInventoryRepositorySqlLite::getAssignedIpsForService(const std::string& servideId) {
    ServiceIpsResult ret;

    std::lock_guard lock(m_dbMutex);

    if (m_db == nullptr) {
        ret.status.error = InventoryError::DbNotInitialized;
        ret.status.detail = "Failed to change service id; reason: database is not initialized";
        return ret;
    }

    // TODO: implement get assigned ips for service.
    (void)servideId;

    return ret;
}

void IpInventoryRepositorySqlLite::clearExpiredReservations() {
    using namespace std::chrono;

    std::lock_guard lock(m_dbMutex);

    if (m_db == nullptr) {
        return;
    }

    SqliteTransaction tx (m_db);

    SqliteStatement deleteExpiredStm(
        m_db,
        R"sql(
            DELETE FROM reserved_ips
            WHERE expiration_time < ?
        )sql"
    );

    const i64 now = i64(system_clock::to_time_t(system_clock::now()));
    deleteExpiredStm.bindInt64(1, now);
    deleteExpiredStm.execute();

    i32 deletedRows = sqlite3_changes(m_db);

    tx.commit();

    std::cout << "Deleted expired reservations count: " << deletedRows << std::endl;
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
            SET reserved_id = ?
            WHERE ip_type = ?
                AND ip_bytes = ?
                AND assigned_id IS NULL
                AND reserved_id IS NULL;
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

void assignIpAddressQuery(sqlite3* db, const IpAddress& address, i64 serviceDbId, i64 reservationId) {
    SqliteStatement dropReservationStm(
        db,
        R"sql(
            DELETE FROM reserved_ips
            WHERE id = ?
        )sql"
    );

    dropReservationStm.bindInt64(1, reservationId);
    dropReservationStm.execute();

    i32 updated = sqlite3_changes(db);
    if (updated != 1) {
        throw std::runtime_error("failed to drop reservation");
    }

    SqliteStatement assignToServiceStm(
        db,
        R"sql(
            UPDATE ip_pool
            SET assigned_id = ?
            WHERE ip_type = ? AND ip_bytes = ?
        )sql"
    );

    assignToServiceStm.bindInt64(1, serviceDbId);
    assignToServiceStm.bindInt(2, i32(address.type));
    assignToServiceStm.bindBlob(3, address.bytes, address.byteCount(address.type));

    assignToServiceStm.execute();

    updated = sqlite3_changes(db);
    if (updated != 1) {
        throw std::runtime_error("assignment failed to update ip_pool table");
    }
}

bool isIpAddressAvailableQuery(sqlite3* db, const IpAddress& address) {
    SqliteStatement findIpAddressStm(
        db,
        R"sql(
            SELECT EXISTS(
                SELECT 1
                FROM ip_pool
                WHERE ip_bytes = ? AND ip_type = ? AND assigned_id == NULL AND reserved_id == NULL
            );
        )sql"
    );

    findIpAddressStm.bindBlob(1, address.bytes, address.byteCount(address.type));
    findIpAddressStm.bindInt(2, i32(address.type));

    if (!findIpAddressStm.stepRow()) {
        return false;
    }

    i32 ret = findIpAddressStm.columnInt(0);
    return ret != 0;
}

bool findAvailableIpAddressQuery(sqlite3* db, IpType type, IpAddress& out) {
    SqliteStatement findServiceStm(
        db,
        R"sql(
            SELECT ip_bytes, display_ip
            FROM ip_pool
            WHERE ip_type = ?
                AND assigned_id IS NULL
                AND reserved_id IS NULL
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

bool findIpAddressReservationQuery(sqlite3* db, const IpAddress& address, Reservation& out) {
    out = {};

    SqliteStatement findReservationIdFromIpAddressStm(
        db,
        R"sql(
            SELECT reserved_id
            FROM ip_pool
            WHERE ip_bytes = ? AND ip_type = ?
        )sql"
    );

    findReservationIdFromIpAddressStm.bindBlob(1, address.bytes, address.byteCount(address.type));
    findReservationIdFromIpAddressStm.bindInt(2, i32(address.type));

    if (!findReservationIdFromIpAddressStm.stepRow()) {
        return false;
    }

    if (findReservationIdFromIpAddressStm.columnIsNull(0)) {
        // No reservation found
        return false;
    }

    i64 reservedId = findReservationIdFromIpAddressStm.columnInt64(0);

    SqliteStatement findReservedIpRecordStm(
        db,
        R"sql(
            SELECT service_id
            FROM reserved_ips
            WHERE id = ?
        )sql"
    );

    findReservedIpRecordStm.bindInt64(1, reservedId);

    if (!findReservedIpRecordStm.stepRow()) {
        return false;
    }

    i64 serviceDbId = findReservedIpRecordStm.columnInt64(0);

    out.reservedId = reservedId;
    out.serviceDbId = serviceDbId;

    return true;
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

i64 findServiceDbIdQuery(sqlite3* db, const std::string& serviceId) {
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
        return -1;
    }

    i64 ret = findServiceStm.columnInt64(0);
    return ret;
}

} // namespace

} // namespace ip_inv
