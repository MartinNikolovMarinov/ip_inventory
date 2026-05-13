#include "handlers.h"

#include "dtos.h"
#include "inventory/inventory_types.h"
#include "str_utils.h"
#include "types.h"

#include <nlohmann/json.hpp>

#include <exception>
#include <format>
#include <fstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace ip_inv {

using json = nlohmann::json;

namespace {

bool parseIpType(const std::string& value, IpType& type);
bool parseIpTypeSelection(const std::string& value, IpTypeSelection& ipTypeSelection);
bool serializeIpType(IpType type, std::string& out);

void setJsonResponse(httplib::Response& response, HttpStatusCode status, const json& body);
template <typename T>
bool parseJsonRequest(const httplib::Request& req, T& dto, std::string& error);

bool toDomainIpAddresses(
    const std::vector<IpAddressDto>& dtoAddresses,
    std::vector<IpAddress>& addresses,
    std::string& error
);

IpAddressesDto toDtoIpAddress(const std::vector<IpAddress>& reservedIps);

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
    if (!toDomainIpAddresses(requestDto.ipAddresses, addresses, error)) {
        setJsonResponse(res, HttpStatusCode::BadRequest, toJson(statusResponse("1", error)));
        return;
    }

    auto result = inventoryService.addIpAddresses(std::move(addresses));
    if (!result.success()) {
        setJsonResponse(res, HttpStatusCode::BadRequest, toJson(statusResponse("1", result.detail)));
        return;
    }

    setJsonResponse(res, HttpStatusCode::Ok, toJson(statusResponse("0", SUCCESSFULL_OPERATION_MSG)));
}

void reserveIpHandler(IpInventoryService& inventoryService, const httplib::Request& req, httplib::Response& res) {
    IpReserveDto requestDto {};
    std::string error;
    if (!parseJsonRequest(req, requestDto, error)) {
        setJsonResponse(res, HttpStatusCode::BadRequest, toJson(statusResponse("1", error)));
        return;
    }

    IpTypeSelection ipTypeSelection;
    if (!parseIpTypeSelection(requestDto.ipType, ipTypeSelection)) {
        setJsonResponse(res, HttpStatusCode::BadRequest, toJson(statusResponse("1", "Failed to parse ip type")));
        return;
    }

    auto result = inventoryService.reserveIpAddress(requestDto.serviceId, ipTypeSelection);
    if (!result.success()) {
        setJsonResponse(res, HttpStatusCode::BadRequest, toJson(statusResponse("1", result.status.detail)));
        return;
    }

    IpAddressesDto responseDto = toDtoIpAddress(result.reservedIps);
    setJsonResponse(res, HttpStatusCode::Ok, toJson(responseDto));
}

void assignIpServiceIdHandler(IpInventoryService& inventoryService, const httplib::Request& req, httplib::Response& res) {
    AssignIpDto requestDto {};
    std::string error;
    if (!parseJsonRequest(req, requestDto, error)) {
        setJsonResponse(res, HttpStatusCode::BadRequest, toJson(statusResponse("1", error)));
        return;
    }

    std::vector<IpAddress> addresses;
    if (!toDomainIpAddresses(requestDto.ipAddresses, addresses, error)) {
        setJsonResponse(res, HttpStatusCode::BadRequest, toJson(statusResponse("1", error)));
        return;
    }

    auto result = inventoryService.assignIpAddress(requestDto.serviceId, std::move(addresses));
    if (!result.success()) {
        setJsonResponse(res, HttpStatusCode::BadRequest, toJson(statusResponse("1", result.detail)));
        return;
    }

    setJsonResponse(res, HttpStatusCode::Ok, toJson(statusResponse("0", SUCCESSFULL_OPERATION_MSG)));
}

void terminateIpServiceIdHandler(IpInventoryService& inventoryService, const httplib::Request& req, httplib::Response& res) {
    TerminateIpDto requestDto {};
    std::string error;
    if (!parseJsonRequest(req, requestDto, error)) {
        setJsonResponse(res, HttpStatusCode::BadRequest, toJson(statusResponse("1", error)));
        return;
    }

    std::vector<IpAddress> addresses;
    if (!toDomainIpAddresses(requestDto.ipAddresses, addresses, error)) {
        setJsonResponse(res, HttpStatusCode::BadRequest, toJson(statusResponse("1", error)));
        return;
    }

    auto result = inventoryService.terminateIpAssignment(requestDto.serviceId, std::move(addresses));
    if (!result.success()) {
        setJsonResponse(res, HttpStatusCode::BadRequest, toJson(statusResponse("1", result.detail)));
        return;
    }

    setJsonResponse(res, HttpStatusCode::Ok, toJson(statusResponse("0", SUCCESSFULL_OPERATION_MSG)));
}

void serviceIdChangeHandler(IpInventoryService& inventoryService, const httplib::Request& req, httplib::Response& res) {
    ChangeServiceDto requestDto {};
    std::string error;
    if (!parseJsonRequest(req, requestDto, error)) {
        setJsonResponse(res, HttpStatusCode::BadRequest, toJson(statusResponse("1", error)));
        return;
    }

    auto result = inventoryService.changeServiceId(requestDto.serviceIdOld, requestDto.serviceId);
    if (!result.success()) {
        setJsonResponse(res, HttpStatusCode::BadRequest, toJson(statusResponse("1", result.detail)));
        return;
    }

    setJsonResponse(res, HttpStatusCode::Ok, toJson(statusResponse("0", SUCCESSFULL_OPERATION_MSG)));
}

void getServiceIdHandler(IpInventoryService& inventoryService, const httplib::Request& req, httplib::Response& res) {
    if (!req.has_param("serviceId")) {
        setJsonResponse(
            res,
            HttpStatusCode::BadRequest,
            toJson(statusResponse("1", "Missing serviceId query parameter"))
        );
        return;
    }

    const std::string serviceId = req.get_param_value("serviceId");

    if (serviceId.empty()) {
        setJsonResponse(
            res,
            HttpStatusCode::BadRequest,
            toJson(statusResponse("1", "serviceId cannot be empty"))
        );
        return;
    }

    auto result = inventoryService.getAssignedIpsForService(serviceId);
    if (!result.success()) {
        setJsonResponse(res, HttpStatusCode::BadRequest, toJson(statusResponse("1", result.status.detail)));
        return;
    }

    IpAddressesDto responseDto = toDtoIpAddress(result.serviceIps);
    setJsonResponse(res, HttpStatusCode::Ok, toJson(responseDto));
}

void serveFileHandler(const char* path, const char* contentType, httplib::Response& res) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        setJsonResponse(res, HttpStatusCode::NotFound, toJson(statusResponse("1", FILE_NOT_FOUND_MSG)));
        return;
    }

    std::string content(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
    );

    res.set_content(std::move(content), contentType);
}

//======================================================================================================================
// Internal helper functions
//======================================================================================================================

namespace {

bool parseIpType(const std::string& value, IpType& type) {
    if (eqIgnoreCase(value, "ipv4")) {
        type = IpType::IPv4;
        return true;
    }
    else if (eqIgnoreCase(value, "ipv6")) {
        type = IpType::IPv6;
        return true;
    }

    return false;
}

bool parseIpTypeSelection(const std::string& value, IpTypeSelection& ipTypeSelection) {
    if (eqIgnoreCase(value, "ipv4")) {
        ipTypeSelection = IpTypeSelection::IPv4;
        return true;
    }
    else if (eqIgnoreCase(value, "ipv6")) {
        ipTypeSelection = IpTypeSelection::IPv6;
        return true;
    }
    else if (eqIgnoreCase(value, "both")) {
        ipTypeSelection = IpTypeSelection::Both;
        return true;
    }

    return false;
}

bool serializeIpType(IpType type, std::string& out) {
    switch (type) {
        case IpType::IPv4:
            out = "IPv4";
            return true;
        case IpType::IPv6:
            out = "IPv6";
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

bool toDomainIpAddresses(
    const std::vector<IpAddressDto>& dtoAddresses,
    std::vector<IpAddress>& addresses,
    std::string& error
) {
    if (dtoAddresses.empty()) {
        error = "Request field 'ipAddresses' must be a non-empty array";
        return false;
    }

    addresses.reserve(dtoAddresses.size());
    for (const IpAddressDto& ipAddressDto : dtoAddresses) {
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

IpAddressesDto toDtoIpAddress(const std::vector<IpAddress>& reservedIps) {
    IpAddressesDto responseDto;
    responseDto.ipAddresses.reserve(reservedIps.size());

    for (const auto& addr : reservedIps) {
        IpAddressDto dto;

        dto.ip = addr.str;
        if (!serializeIpType(addr.type, dto.ipType)) {
            throw std::runtime_error("failed to serialize response");
        }

        responseDto.ipAddresses.push_back(std::move(dto));
    }

    return responseDto;
}

} // namespace

} // namespace ip_inv
