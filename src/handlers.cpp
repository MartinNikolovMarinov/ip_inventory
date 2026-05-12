#include "handlers.h"

#include "dtos.h"
#include "types.h"

#include <nlohmann/json.hpp>

#include <exception>
#include <format>
#include <string>
#include <utility>
#include <vector>

namespace ip_inv {

using json = nlohmann::json;

namespace {

bool parseIpType(const std::string& value, IpType& type);

void setJsonResponse(httplib::Response& response, HttpStatusCode status, const json& body);
template <typename T>
bool parseJsonRequest(const httplib::Request& req, T& dto, std::string& error);

bool toDomainIpAddresses(const IpAddressesDto& dto, std::vector<IpAddress>& addresses, std::string& error);

} // namespace

//======================================================================================================================
// PUBLIC
//======================================================================================================================

void addIpPoolHandler(IpInventoryService& inventoryService, const httplib::Request& req, httplib::Response& res) {
    IpAddressesDto requestDto {};
    std::string error;
    if (!parseJsonRequest(req, requestDto, error)) {
        setJsonResponse(res, HttpStatusCode::BadRequest, toJson(statusResponse("1", error)));
        return;
    }

    std::vector<IpAddress> addresses;
    if (!toDomainIpAddresses(requestDto, addresses, error)) {
        setJsonResponse(res, HttpStatusCode::BadRequest, toJson(statusResponse("1", error)));
        return;
    }

    AddToPoolResult result = inventoryService.addIpAddresses(std::move(addresses));
    if (!result.success()) {
        setJsonResponse(res, HttpStatusCode::BadRequest, toJson(statusResponse("1", result.status.detail)));
        return;
    }

    setJsonResponse(res, HttpStatusCode::Ok, toJson(statusResponse("0", "Successful operation. OK")));
}

void serveFile(const char* path, const char* contentType, httplib::Response& response) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        response.status = i32(HttpStatusCode::NotFound);
        response.set_content(R"({"status":"not found"})", "application/json");
        return;
    }

    std::string content(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
    );

    response.set_content(std::move(content), contentType);
}

//======================================================================================================================
// Internal helper functions
//======================================================================================================================

namespace {

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

void setJsonResponse(httplib::Response& response, HttpStatusCode status, const json& body) {
    response.status = i32(status);
    response.set_content(body.dump(), "application/json");
}

template <typename T>
bool parseJsonRequest(const httplib::Request& req, T& dto, std::string& error) {
    const json body = json::parse(req.body, nullptr, false);
    if (body.is_discarded() || !body.is_object()) {
        error = "Invalid JSON request body";
        return false;
    }

    try {
        fromJson(body, dto);
    }
    catch (const std::exception& ex) {
        error = std::format("Invalid JSON request body: {}", ex.what());
        return false;
    }

    return true;
}

bool toDomainIpAddresses(const IpAddressesDto& dto, std::vector<IpAddress>& addresses, std::string& error) {
    if (dto.ipAddresses.empty()) {
        error = "Request field 'ipAddresses' must be a non-empty array";
        return false;
    }

    addresses.reserve(dto.ipAddresses.size());
    for (const IpAddressDto& ipAddressDto : dto.ipAddresses) {
        IpAddress address {};
        address.str = ipAddressDto.ip;
        if (!parseIpType(ipAddressDto.ipType, address.type)) {
            error = std::format("Invalid ipType: {}", ipAddressDto.ipType);
            return false;
        }

        addresses.push_back(std::move(address));
    }

    return true;
}

} // namespace

} // namespace ip_inv
