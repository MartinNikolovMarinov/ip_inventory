#pragma once

#include "types.h"

#include <memory>
#include <string>

namespace ip_inv {

struct AppConfig {
    std::string ipAddress;
    i32 port;
    usize serverThreadCount = 4;
    usize gcIntervalSeconds = 60;
};

class App {
public:
    struct Impl;

private:
    App(std::unique_ptr<Impl> impl);

public:
    ~App();

    App(const App&) = delete;
    App& operator=(const App&) = delete;
    App(App&&) noexcept;
    App& operator=(App&&) noexcept;

    static App create(AppConfig&& config);

    [[nodiscard]] i32 run();
    void shutdown();

private:
    std::unique_ptr<Impl> m_impl;
};

} // namespace ip_inv
