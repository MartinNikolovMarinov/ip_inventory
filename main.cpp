#include "types.h"

#include <drogon/orm/DbClient.h>

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace ip_inv;

// namespace {

// std::string source_path(const char *relative_path) {
//     return (std::filesystem::path(IP_INVENTORY_SOURCE_DIR) / relative_path).string();
// }

// drogon::HttpResponsePtr text_response(std::string body, std::string content_type) {
//     auto response = drogon::HttpResponse::newHttpResponse();
//     response->setBody(std::move(body));
//     response->setContentTypeString(content_type);
//     return response;
// }

// } // namespace

// i32 main() {
//     auto &app = drogon::app();

//     app.registerHandler(
//         "/health",
//         [](const drogon::HttpRequestPtr &,
//            std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
//             callback(text_response(R"({"status":"ok"})", "application/json"));
//         },
//         {drogon::Get});

//     app.registerHandler(
//         "/openapi.yaml",
//         [](const drogon::HttpRequestPtr &,
//            std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
//             callback(drogon::HttpResponse::newFileResponse(
//                 source_path("api/openapi.yaml"), "", drogon::CT_CUSTOM, "application/yaml"));
//         },
//         {drogon::Get});

//     const auto serve_swagger_docs =
//         [](const drogon::HttpRequestPtr &,
//            std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
//             callback(drogon::HttpResponse::newFileResponse(
//                 source_path("api/openapi-index.html"), "", drogon::CT_TEXT_HTML));
//         };

//     app.registerHandler("/docs", serve_swagger_docs, {drogon::Get});
//     app.registerHandler("/docs/", serve_swagger_docs, {drogon::Get});

//     app.setThreadNum(4)
//         .addListener("0.0.0.0", 8080)
//         .addALocation("/swagger-ui-assets", "", source_path("vendor/swagger-ui/dist"));

//     std::cout << "IP Inventory API listening on http://0.0.0.0:8080\n";
//     std::cout << "Swagger UI available at http://localhost:8080/docs/\n";
//     // app.run();
//     return 0;
// }

i32 main() {
    // const char* connectionString = std::getenv("IP_INVENTORY_DB");
    // if (connectionString == nullptr || connectionString[0] == '\0') {
    //     std::cerr << "IP_INVENTORY_DB is not set\n";
    //     return 1;
    // }

    // IP_INVENTORY_DB="host=127.0.0.1 port=5432 dbname=ip_inventory user=postgres password=postgres"

    const char* connectionString = "host=127.0.0.1 port=5432 dbname=ip_inventory user=postgres password=postgres";

    try {
        auto db = drogon::orm::DbClient::newPgClient(connectionString, 1);

        const std::string ip = "203.0.113.10";
        const std::string ipType = "IPv4";

        db->execSqlSync(
            "insert into ip_pool (ip, ip_type) values (?, ?) "
            "on conflict (ip) do nothing",
            ip,
            ipType);

        const auto selected = db->execSqlSync(
            "select ip, ip_type, state from ip_pool where ip = ?",
            ip);

        for (const auto& row : selected) {
            std::cout << "Selected IP: "
                      << row["ip"].as<std::string>() << " "
                      << row["ip_type"].as<std::string>() << " "
                      << row["state"].as<std::string>() << '\n';
        }

        db->execSqlSync("delete from ip_pool where ip = ?", ip);
        std::cout << "Deleted demo IP: " << ip << '\n';

        return 0;
    }
    catch (const std::exception& error) {
        std::cerr << "Database demo failed: " << error.what() << '\n';
        return 1;
    }
}
