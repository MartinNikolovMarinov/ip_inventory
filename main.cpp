#include "types.h"

// #include <drogon/drogon.h>

#include <filesystem>
#include <iostream>
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

i32 main() {
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

    std::cout << "IP Inventory API listening on http://0.0.0.0:8080\n";
    std::cout << "Swagger UI available at http://localhost:8080/docs/\n";

//     // app.run();
    return 0;
}
