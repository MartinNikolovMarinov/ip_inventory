#pragma once

#include "types.h"

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
    std::vector<IpAddressDto> ipAddresses;
};

struct TerminateIpDto {
    std::string serviceId;
    std::vector<IpAddressDto> ipAddresses;
};

struct ChangeServiceDto {
    std::string serviceOld;
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

} // namespace ip_inv
