#include "app.h"

#include <thread>
#include <iostream>

using namespace ip_inv;

// TODO: add graceful shutdown on signal interupt.

i32 main() {
    const u32 concurrency = std::thread::hardware_concurrency() / 2;

    AppConfig cfg;
    cfg.ipAddress = "localhost";
    cfg.databaseName = "ip_inventory.sqlite3";
    cfg.schemaInitScriptPath = IP_INVENTORY_SOURCE_DIR "/schema/001_init_db.sql";
    cfg.dropCreateDbOnStart = false;
    cfg.port = 8080;
    cfg.serverThreadCount = concurrency > 0 ? concurrency : 1;
    cfg.gcIntervalSeconds = 60;
    cfg.reservationExpirationSeconds = 10;

    App app = App::create(std::move(cfg));
    i32 returnCode = app.run();

    std::cout << "Application exit code = " << returnCode << std::endl;

    return returnCode;
}
