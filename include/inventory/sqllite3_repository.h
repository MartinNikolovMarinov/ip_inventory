#pragma once

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

    void initializeDb() override;

    [[nodiscard]] AddToPoolResult addIpAddresses(const std::vector<IpAddress>& addresses) override;

private:
    std::string m_databaseName;
    std::filesystem::path m_databasePath;
    sqlite3* m_db = nullptr;
    std::mutex m_dbMutex;
};

} // namespace ip_inv
