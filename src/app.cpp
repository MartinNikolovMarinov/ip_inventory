#include "app.h"
#include "ip_utils.h"
#include "validation.h"

#include "inventory/sqllite3_repository.h"
#include "inventory/service.h"

#include <httplib.h>
#include <nlohmann/json.hpp>

#include <chrono>
#include <condition_variable>
#include <format>
#include <iostream>
#include <iterator>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

namespace ip_inv {

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

    void shutdown() {
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

namespace {

using json = nlohmann::json;

bool parseIpType(const std::string& value, IpType& type) {
    if (value == "IPv4") {
        type = IpType::IPv4;
        return true;
    }
    if (value == "IPv6") {
        type = IpType::IPv6;
        return true;
    }

    return false;
}

json statusResponse(const char* statusCode, const char* statusMessage) {
    return json {
        {"statusCode", statusCode},
        {"statusMessage", statusMessage},
    };
}

void setJsonResponse(httplib::Response& response, int status, const json& body) {
    response.status = status;
    response.set_content(body.dump(), "application/json");
}

bool parseAddIpPoolRequest(const httplib::Request& request, std::vector<IpAddress>& addresses, std::string& error) {
    const json body = json::parse(request.body, nullptr, false);
    if (body.is_discarded() || !body.is_object()) {
        error = "Invalid JSON request body";
        return false;
    }

    const auto ipAddresses = body.find("ipAddresses");
    if (ipAddresses == body.end() || !ipAddresses->is_array() || ipAddresses->empty()) {
        error = "Request field 'ipAddresses' must be a non-empty array";
        return false;
    }

    for (const json& item : *ipAddresses) {
        if (!item.is_object()) {
            error = "Each ipAddresses entry must be an object";
            return false;
        }

        const auto ip = item.find("ip");
        if (ip == item.end() || !ip->is_string()) {
            error = "Each ipAddresses entry must contain string field 'ip'";
            return false;
        }

        const auto ipType = item.find("ipType");
        if (ipType == item.end() || !ipType->is_string()) {
            error = "Each ipAddresses entry must contain string field 'ipType'";
            return false;
        }

        IpAddress address {};
        address.value = ip->get<std::string>();
        if (!parseIpType(ipType->get<std::string>(), address.type)) {
            error = std::format("Invalid ipType: {}", ipType->get<std::string>());
            return false;
        }

        addresses.push_back(std::move(address));
    }

    return true;
}

void configureHttpRoutes(App::Impl& app) {
    app.server.Get("/health", [&app](const httplib::Request&, httplib::Response& response) {
        std::cout << "Called /health endpoint" << std::endl;
        response.set_content(R"({"status":"ok"})", "application/json");
        app.shutdown();
    });

    app.server.Post("/ip-inventory/ip-pool", [&app](const httplib::Request& request, httplib::Response& response) {
        std::vector<IpAddress> addresses;
        std::string error;
        if (!parseAddIpPoolRequest(request, addresses, error)) {
            setJsonResponse(response, 400, statusResponse("1", error.c_str()));
            return;
        }

        AddToPoolResult result = app.inventoryService->addIpAddresses(addresses);
        if (!result.success()) {
            setJsonResponse(response, 400, statusResponse("1", result.status.detail.c_str()));
            return;
        }

        setJsonResponse(response, 200, statusResponse("0", "Successful operation. OK"));
    });
}

void validateAppConfig(const AppConfig& config) {
    if (!isValidPort(config.port)) {
        throw std::invalid_argument(std::format("invalid port: {}", config.port));
    }
    if (!isValidDatabaseName(config.databaseName)) {
        throw std::invalid_argument(std::format("invalid database name: {}", config.databaseName));
    }
    IpAddress ipAddress {};
    if (!parseIpV4(config.ipAddress, ipAddress)) {
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
    inventoryRepository->initializeDb();
    impl->inventoryService = std::make_unique<IpInventoryService>(std::move(inventoryRepository));

    App app(std::move(impl));
    std::cout << "Application created successfully" << std::endl;
    return app;
}

i32 App::run() {
    std::cout << "Application start on "
              << m_impl->cfg.ipAddress << ":" << m_impl->cfg.port
              << std::endl;

    // Start garbage collection thread:
    m_impl->gcThread = std::thread([this] {
        auto gcIntervalSeconds = std::chrono::seconds(m_impl->cfg.gcIntervalSeconds);

        while (true) {
            {
                // Wait for interval seconds, or until a shutdown signal was triggered.
                std::unique_lock lock(m_impl->shutdownMutex);
                const bool shouldStop = m_impl->shutdownCondition.wait_for(lock, gcIntervalSeconds, [this] {
                    return m_impl->shutdownInProgress || m_impl->serverStopped;
                });
                if (shouldStop) {
                    break;
                }
            }

            std::cout << "gc triggered" << '\n';
        }
    });

    // Start the server thread
    m_impl->serverThread = std::thread([this] {
        bool ok = m_impl->server.listen(m_impl->cfg.ipAddress, m_impl->cfg.port);

        {
            std::lock_guard lock(m_impl->shutdownMutex);
            m_impl->failedToStart = ok;
            m_impl->serverStopped = true;
        }

        m_impl->shutdownCondition.notify_one();
    });

    // Block until a shutdown signal is raised:
    {
        std::unique_lock lock(m_impl->shutdownMutex);
        m_impl->shutdownCondition.wait(lock, [this] {
            return m_impl->shutdownInProgress || m_impl->serverStopped;
        });
    }

    m_impl->server.stop();

    // Join spawned background threads.
    {
        if (m_impl->serverThread.joinable()) {
            m_impl->serverThread.join();
        }
        if (m_impl->gcThread.joinable()) {
            m_impl->gcThread.join();
        }
    }

    i32 returnCode = (m_impl->shutdownInProgress || m_impl->failedToStart) ? 0 : 1;
    return returnCode;
}

void App::shutdown() {
    std::cout << "Application shuttingdown" << std::endl;

    {
        std::lock_guard lock(m_impl->shutdownMutex);
        if (m_impl->shutdownInProgress) {
            return;
        }
        m_impl->shutdownInProgress = true;
    }

    m_impl->shutdownCondition.notify_one();
}

} // namespace ip_inv
