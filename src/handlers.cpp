#include "handlers.h"

#include "dtos.h"

#include <nlohmann/json.hpp>

#include <exception>
#include <format>
#include <string>
#include <utility>
#include <vector>

namespace ip_inv {

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

StatusResponseDto statusResponse(std::string statusCode, std::string statusMessage) {
    return StatusResponseDto {
        .statusCode = std::move(statusCode),
        .statusMessage = std::move(statusMessage),
    };
}

void setJsonResponse(httplib::Response& response, int status, const json& body) {
    response.status = status;
    response.set_content(body.dump(), "application/json");
}

template <typename T>
bool parseJsonRequest(const httplib::Request& request, T& dto, std::string& error) {
    const json body = json::parse(request.body, nullptr, false);
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
        address.value = ipAddressDto.ip;
        if (!parseIpType(ipAddressDto.ipType, address.type)) {
            error = std::format("Invalid ipType: {}", ipAddressDto.ipType);
            return false;
        }

        addresses.push_back(std::move(address));
    }

    return true;
}

} // namespace

void addIpPoolHandler(IpInventoryService& inventoryService, const httplib::Request& req, httplib::Response& res) {
    IpAddressesDto requestDto {};
    std::string error;
    if (!parseJsonRequest(req, requestDto, error)) {
        setJsonResponse(res, 400, toJson(statusResponse("1", error)));
        return;
    }

    std::vector<IpAddress> addresses;
    if (!toDomainIpAddresses(requestDto, addresses, error)) {
        setJsonResponse(res, 400, toJson(statusResponse("1", error)));
        return;
    }

    AddToPoolResult result = inventoryService.addIpAddresses(addresses);
    if (!result.success()) {
        setJsonResponse(res, 400, toJson(statusResponse("1", result.status.detail)));
        return;
    }

    setJsonResponse(res, 200, toJson(statusResponse("0", "Successful operation. OK")));
}

} // namespace ip_inv
