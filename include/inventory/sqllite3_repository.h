#pragma once

#include "inventory/inventory_types.h"
#include "inventory/repository.h"

#include <filesystem>
#include <mutex>
#include <string>

struct sqlite3;

namespace ip_inv {

class IpInventoryRepositorySqlLite : public IpInventoryRepository {
public:
    explicit IpInventoryRepositorySqlLite(std::string databaseName);
    ~IpInventoryRepositorySqlLite() noexcept override;

    void initializeDb(bool dropCreate = false, std::filesystem::path schemaInitScriptPath = {}) override;

    [[nodiscard]] InventoryStatus addIpAddresses(const std::vector<IpAddress>& addresses) override;

    [[nodiscard]] ReserveIpResult reserveIpAddress(
        const std::string& serviceId,
        IpTypeSelection ipTypeSection,
        i64 expirationTime
    ) override;

    [[nodiscard]] InventoryStatus assignIpAddress(
        const std::string& serviceId,
        std::vector<IpAddress>&& addresses
    ) override;

    [[nodiscard]] InventoryStatus terminateIpAssignment(
        const std::string& serviceId,
        std::vector<IpAddress>&& addresses
    ) override;

    [[nodiscard]] virtual InventoryStatus changeServiceId(
        const std::string& servideIdOld,
        const std::string& serviceIdNew
    ) override;

    [[nodiscard]] virtual ServiceIpsResult getAssignedIpsForService(const std::string& servideId) override;

    [[nodiscard]] ReservedIpsResult getReservedIps() override;

    void clearExpiredReservations() override;

private:
    std::string m_databaseName;
    std::filesystem::path m_databasePath;
    sqlite3* m_db = nullptr;
    std::mutex m_dbMutex;
};

} // namespace ip_inv
