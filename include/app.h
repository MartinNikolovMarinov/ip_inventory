#pragma once

#include "types.h"

#include <memory>
#include <string>

namespace ip_inv {

class IpInventoryRepository;
class IpInventoryService;

struct AppConfig {
    std::string ipAddress;
    i32 port;
};

class App {
private:
    App();

public:
    ~App();

    App(App&&) noexcept;
    App& operator=(App&&) noexcept;

    static App create(AppConfig&& config);

    void start();

private:
    AppConfig m_cfg;
    std::unique_ptr<IpInventoryService> m_inventoryService;
};

} // namespace ip_inv
