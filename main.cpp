#include "app.h"

#include <filesystem>
#include <iostream>
#include <thread>

using namespace ip_inv;

// TODO: add graceful shutdown on signal interupt.

namespace {

constexpr usize defaultReservationExpirationSeconds() {
#if defined(IP_INVENTORY_RESERVATION_EXPIRATION_SECONDS)
    return IP_INVENTORY_RESERVATION_EXPIRATION_SECONDS;
#else
    return 20 * 60;
#endif
}

} // namespace

i32 main() {
    const u32 concurrency = std::thread::hardware_concurrency() / 2;

    AppConfig cfg;
    cfg.ipAddress = IP_INVENTORY_LISTEN_ADDRESS;
    cfg.databaseName = "ip_inventory.sqlite3";
    cfg.schemaInitScriptPath = IP_INVENTORY_SOURCE_DIR "/schema/001_init_db.sql";
    cfg.dropCreateDbOnStart = !std::filesystem::exists(
        std::filesystem::path(IP_INVENTORY_SQLITE3_DB_ROOT) / cfg.databaseName
    );
    cfg.port = 8080;
    cfg.serverThreadCount = concurrency > 0 ? concurrency : 1;
    cfg.gcIntervalSeconds = 60 * 60;
    cfg.reservationExpirationSeconds = defaultReservationExpirationSeconds();

    std::cout << "Reservation expiration seconds = " << cfg.reservationExpirationSeconds << std::endl;

    App app = App::create(std::move(cfg));
    i32 returnCode = app.run();

    std::cout << "Application exit code = " << returnCode << std::endl;

    return returnCode;
}
