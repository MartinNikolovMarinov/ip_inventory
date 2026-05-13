#pragma once

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

namespace ip_inv {

struct IpAddressDto {
    std::string ip;
    std::string ipType;
};

struct IpReserveDto {
    std::string serviceId;
    std::string ipType;
};

struct IpAddressesDto {
    std::vector<IpAddressDto> ipAddresses;
};

struct StatusResponseDto {
    std::string statusCode;
    std::string statusMessage;
};

struct AssignIpDto {
    std::string serviceId;
    std::vector<std::string> ipAddresses;
};

struct TerminateIpDto {
    std::string serviceId;
    std::vector<std::string> ipAddresses;
};

struct ChangeServiceDto {
    std::string serviceIdOld;
    std::string serviceId;
};

[[nodiscard]] nlohmann::json toJson(const IpAddressDto& dto);
void fromJson(const nlohmann::json& json, IpAddressDto& dto);

[[nodiscard]] nlohmann::json toJson(const IpReserveDto& dto);
void fromJson(const nlohmann::json& json, IpReserveDto& dto);

[[nodiscard]] nlohmann::json toJson(const IpAddressesDto& dto);
void fromJson(const nlohmann::json& json, IpAddressesDto& dto);

[[nodiscard]] nlohmann::json toJson(const StatusResponseDto& dto);
void fromJson(const nlohmann::json& json, StatusResponseDto& dto);

[[nodiscard]] nlohmann::json toJson(const AssignIpDto& dto);
void fromJson(const nlohmann::json& json, AssignIpDto& dto);

[[nodiscard]] nlohmann::json toJson(const TerminateIpDto& dto);
void fromJson(const nlohmann::json& json, TerminateIpDto& dto);

[[nodiscard]] nlohmann::json toJson(const ChangeServiceDto& dto);
void fromJson(const nlohmann::json& json, ChangeServiceDto& dto);

constexpr inline StatusResponseDto statusResponse(std::string statusCode, std::string statusMessage) {
    return StatusResponseDto { std::move(statusCode), std::move(statusMessage) };
}

constexpr const char* SUCCESSFULL_OPERATION_MSG = "Successful operation. OK";
constexpr const char* FILE_NOT_FOUND_MSG = "file not found";

} // namespace ip_inv
