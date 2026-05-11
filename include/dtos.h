#pragma once

#include "types.h"

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
    i32 statusCode;
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

} // namespace ip_inv
