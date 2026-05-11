#include "app.h"
#include "validation.h"

#include "inventory/sqllite3_repository.h"
#include "inventory/service.h"

#include <httplib.h>
#include <sqlite3.h>

#include <format>
#include <memory>
#include <stdexcept>
#include <utility>

namespace ip_inv {

App::App() = default;
App::~App() = default;

App::App(App&&) noexcept = default;
App& App::operator=(App&&) noexcept = default;

App App::create(AppConfig&& config) {
    if (!isValidPort(config.port)) {
        throw std::invalid_argument(std::format("invalid port: {}", config.port));
    }
    if (config.ipAddress == "localhost") {
        config.ipAddress = "127.0.0.1";
    }
    if (!isValidIpv4Address(config.ipAddress.data(), config.ipAddress.length())) {
        throw std::invalid_argument(std::format("invalid ip address: {}", config.ipAddress));
    }

    App app;

    app.m_cfg = std::move(config);

    auto inventoryRepository = std::make_unique<IpInventoryRepositorySqlLite>();
    app.m_inventoryService = std::make_unique<IpInventoryService>(std::move(inventoryRepository));

    return app;
}

void App::start() {
    // TODO: start the app..
}

} // namespace ip_inv
