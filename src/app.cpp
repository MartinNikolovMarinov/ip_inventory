#include "app.h"
#include "dtos.h"
#include "handlers.h"
#include "ip_utils.h"
#include "types.h"
#include "validation.h"
#include "profiling.h"

#include "inventory/sqllite3_repository.h"
#include "inventory/service.h"

#include <httplib.h>

#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <format>
#include <iostream>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <utility>

namespace ip_inv {

namespace {

void configureHttpRoutes(App::Impl& app);

void logEndpointCall(const std::string_view endpoint, const httplib::Request& req);
void registerStaticFileRoute(
    httplib::Server& server,
    const char* endpoint,
    const char* path,
    const char* contentType
);

template<typename Func>
void endpointGuard(
    const char* endpoint,
    const httplib::Request& req,
    httplib::Response& response,
    Func&& func
);

void validateAppConfig(const AppConfig& config);

} // namespace

struct App::Impl {
    AppConfig cfg;
    httplib::Server server;
    std::unique_ptr<IpInventoryService> inventoryService;

    mutable std::mutex shutdownMutex;
    mutable std::condition_variable shutdownCondition;
    bool shutdownInProgress = false;
    bool serverStopped = false;
    bool failedToStart = false;
    std::thread serverThread;
    std::thread gcThread;

    i32 run() {
        std::cout << "Application start on " << cfg.ipAddress << ":" << cfg.port << std::endl;

        // Start garbage collection thread:
        gcThread = std::thread([this] {
            auto gcIntervalSeconds = std::chrono::seconds(cfg.gcIntervalSeconds);

            while (true) {
                {
                    // Wait for interval seconds, or until a shutdown signal was triggered.
                    std::unique_lock lock(shutdownMutex);
                    const bool shouldStop = shutdownCondition.wait_for(lock, gcIntervalSeconds, [this] {
                        return shutdownInProgress || serverStopped;
                    });
                    if (shouldStop) {
                        break;
                    }
                }

                std::cout << "gc triggered" << '\n';
                // TODO: Run GC
            }
        });

        // Start the server thread
        serverThread = std::thread([this] {
            bool ok = server.listen(cfg.ipAddress, cfg.port);

            {
                std::lock_guard lock(shutdownMutex);
                failedToStart = ok;
                serverStopped = true;
            }

            shutdownCondition.notify_one();
        });

        // Block until a shutdown signal is raised:
        {
            std::unique_lock lock(shutdownMutex);
            shutdownCondition.wait(lock, [this] {
                return shutdownInProgress || serverStopped;
            });
        }

        server.stop();

        // Join spawned background threads.
        {
            if (serverThread.joinable()) {
                serverThread.join();
            }
            if (gcThread.joinable()) {
                gcThread.join();
            }
        }

        i32 returnCode = (shutdownInProgress || failedToStart) ? 0 : 1;
        return returnCode;
    }

    void shutdown() {
        std::cout << "Application shuttingdown" << std::endl;

        {
            std::lock_guard lock(shutdownMutex);
            if (shutdownInProgress) {
                return;
            }
            shutdownInProgress = true;
        }

        shutdownCondition.notify_one();
    }
};

//======================================================================================================================
// PUBLIC
//======================================================================================================================

App::App(std::unique_ptr<Impl> impl) : m_impl(std::move(impl)) {}

App::~App() = default;

App::App(App&&) noexcept = default;
App& App::operator=(App&&) noexcept = default;

App App::create(AppConfig&& config) {
    std::cout << "Creating application" << std::endl;

    if (config.ipAddress == "localhost") {
        config.ipAddress = "127.0.0.1";
    }
    validateAppConfig(config);

    auto impl = std::make_unique<Impl>();
    impl->cfg = std::move(config);
    impl->server.new_task_queue = [threadCount = impl->cfg.serverThreadCount] {
        return new httplib::ThreadPool(threadCount);
    };
    configureHttpRoutes(*impl);

    auto inventoryRepository = std::make_unique<IpInventoryRepositorySqlLite>(impl->cfg.databaseName);
    inventoryRepository->initializeDb(impl->cfg.dropCreateDbOnStart, impl->cfg.schemaInitScriptPath);
    impl->inventoryService = std::make_unique<IpInventoryService>(std::move(inventoryRepository));

    App app(std::move(impl));
    std::cout << "Application created successfully" << std::endl;
    return app;
}

i32 App::run() {
    return m_impl->run();
}

void App::shutdown() {
    m_impl->shutdown();
}

//======================================================================================================================
// Internal helper functions
//======================================================================================================================

namespace {

void configureHttpRoutes(App::Impl& app) {
    constexpr const char* HEALTH_ENDPOINT = "/health";
    constexpr const char* IP_POOL_ENDPOINT = "/ip-inventory/ip-pool";
    constexpr const char* DOCS_NO_TRAILING_SLASH_ENDPOINT = "/docs";
    constexpr const char* DOCS_ENDPOINT = "/docs/";
    constexpr const char* OPENAPI_YAML_ENDPOINT = "/openapi.yaml";

    app.server.Get(HEALTH_ENDPOINT, [](const httplib::Request& req, httplib::Response& res) {
        endpointGuard(HEALTH_ENDPOINT, req, res, [&] {
            auto statusDto = statusResponse("0", "healthy");
            res.status = i32(HttpStatusCode::Ok);
            res.set_content(toJson(statusDto).dump(), "application/json");
        });
    });
    app.server.Post("/ip-inventory/ip-pool", [&app](const httplib::Request& req, httplib::Response& res) {
        endpointGuard(IP_POOL_ENDPOINT, req, res, [&] {
            addIpPoolHandler(*app.inventoryService, req, res);
        });
    });

    app.server.set_mount_point(
        "/swagger-ui-assets",
        IP_INVENTORY_SOURCE_DIR "/api/swagger-ui"
    );
    app.server.Get(DOCS_ENDPOINT, [](const httplib::Request& req, httplib::Response& res) {
        endpointGuard(DOCS_ENDPOINT, req, res, [&] {
            serveFile(IP_INVENTORY_SOURCE_DIR "/api/openapi.html", "text/html", res);
        });
    });
    app.server.Get(DOCS_NO_TRAILING_SLASH_ENDPOINT, [](const httplib::Request& req, httplib::Response& res) {
        endpointGuard(DOCS_NO_TRAILING_SLASH_ENDPOINT, req, res, [&] {
            serveFile(IP_INVENTORY_SOURCE_DIR "/api/openapi.html", "text/html", res);
        });
    });
    app.server.Get(OPENAPI_YAML_ENDPOINT, [](const httplib::Request& req, httplib::Response& res) {
        endpointGuard(OPENAPI_YAML_ENDPOINT, req, res, [&] {
            serveFile(IP_INVENTORY_SOURCE_DIR "/api/openapi.yaml", "application/yaml", res);
        });
    });
}

void logEndpointCall(const std::string_view endpoint, const httplib::Request& req) {
    std::cout
        << "[HTTP] endpoint=" << endpoint
        << " | method=" << req.method
        << " | path=" << req.path
        << " | remote=" << req.remote_addr << ':' << req.remote_port;

    if (!req.params.empty()) {
        std::cout << " | params={";

        i32 i = 0;
        for (const auto& [key, value] : req.params) {
            if (i > 0) std::cout << ", ";
            std::cout << key << '=' << value;
            i++;
        }

        std::cout << '}';
    }

    std::cout << std::endl;
}

template<typename Func>
void endpointGuard(
    const char* endpoint,
    const httplib::Request& req,
    httplib::Response& response,
    Func&& func
) {
    // TODO: Add cheap request-level safety checks before handlers parse or allocate:
    //       * reject request bodies above the endpoint's expected maximum size
    //       * require application/json for JSON endpoints
    //       * keep DTO/domain validation strict: bounded arrays, bounded strings, known fields, known enum values
    //       * keep SQL injection resistance in repository code by using prepared statements and bound parameters only

    try {
        ScopeProfiler profiler(endpoint);

        logEndpointCall(endpoint, req);

        std::forward<Func>(func)();
    }
    catch (const std::exception& e) {
        std::cerr
            << "[ERROR]"
            << " | endpoint=" << endpoint
            << " | exception=" << e.what()
            << std::endl;

        auto statusDto = statusResponse("1", "internal server error");
        response.status = i32(HttpStatusCode::InternalServerError);
        response.set_content(toJson(statusDto).dump(), "application/json");
    }
    catch (...) {
        std::cerr
            << "[ERROR]"
            << " | endpoint=" << endpoint
            << " | exception=unknown"
            << std::endl;
        auto statusDto = statusResponse("1", "internal server error");
        response.status = i32(HttpStatusCode::InternalServerError);
        response.set_content(toJson(statusDto).dump(), "application/json");
    }
}

void validateAppConfig(const AppConfig& config) {
    if (!isValidPort(config.port)) {
        throw std::invalid_argument(std::format("invalid port: {}", config.port));
    }
    if (!isValidDatabaseName(config.databaseName)) {
        throw std::invalid_argument(std::format("invalid database name: {}", config.databaseName));
    }
    if (config.dropCreateDbOnStart) {
        if (config.schemaInitScriptPath.empty()) {
            throw std::invalid_argument("schema init script path must not be empty when drop-create is enabled");
        }
        if (!std::filesystem::is_regular_file(config.schemaInitScriptPath)) {
            throw std::invalid_argument(std::format("schema init script does not exist: {}", config.schemaInitScriptPath));
        }
    }
    IpAddress ipAddress {};
    ipAddress.str = config.ipAddress;
    if (!parseIpV4(ipAddress)) {
        throw std::invalid_argument(std::format("invalid ip address: {}", config.ipAddress));
    }
    if (config.serverThreadCount == 0) {
        throw std::invalid_argument("server thread count must be greater than zero");
    }
    if (config.gcIntervalSeconds == 0) {
        throw std::invalid_argument("gc interval seconds must be greater than zero");
    }
}

} // namespace

} // namespace ip_inv
