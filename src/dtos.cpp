#include "dtos.h"

#include <utility>

namespace ip_inv {

nlohmann::json toJson(const IpAddressDto& dto) {
    return nlohmann::json {
        {"ip", dto.ip},
        {"ipType", dto.ipType},
    };
}

void fromJson(const nlohmann::json& json, IpAddressDto& dto) {
    json.at("ip").get_to(dto.ip);
    json.at("ipType").get_to(dto.ipType);
}

nlohmann::json toJson(const IpReserveDto& dto) {
    return nlohmann::json {
        {"serviceId", dto.serviceId},
        {"ipType", dto.ipType},
    };
}

void fromJson(const nlohmann::json& json, IpReserveDto& dto) {
    json.at("serviceId").get_to(dto.serviceId);
    json.at("ipType").get_to(dto.ipType);
}

nlohmann::json toJson(const IpAddressesDto& dto) {
    nlohmann::json ipAddresses = nlohmann::json::array();
    for (const IpAddressDto& ipAddress : dto.ipAddresses) {
        ipAddresses.push_back(toJson(ipAddress));
    }

    return nlohmann::json {
        {"ipAddresses", std::move(ipAddresses)},
    };
}

void fromJson(const nlohmann::json& json, IpAddressesDto& dto) {
    const nlohmann::json& ipAddresses = json.at("ipAddresses");
    dto.ipAddresses.clear();
    dto.ipAddresses.reserve(ipAddresses.size());

    for (const nlohmann::json& item : ipAddresses) {
        IpAddressDto ipAddress {};
        fromJson(item, ipAddress);
        dto.ipAddresses.push_back(std::move(ipAddress));
    }
}

nlohmann::json toJson(const StatusResponseDto& dto) {
    return nlohmann::json {
        {"statusCode", dto.statusCode},
        {"statusMessage", dto.statusMessage},
    };
}

void fromJson(const nlohmann::json& json, StatusResponseDto& dto) {
    json.at("statusCode").get_to(dto.statusCode);
    json.at("statusMessage").get_to(dto.statusMessage);
}

nlohmann::json toJson(const AssignIpDto& dto) {
    nlohmann::json ipAddresses = nlohmann::json::array();
    for (const IpAddressDto& ipAddress : dto.ipAddresses) {
        ipAddresses.push_back(toJson(ipAddress));
    }

    return nlohmann::json {
        {"serviceId", dto.serviceId},
        {"ipAddresses", std::move(ipAddresses)},
    };
}

void fromJson(const nlohmann::json& json, AssignIpDto& dto) {
    json.at("serviceId").get_to(dto.serviceId);
    const nlohmann::json& ipAddresses = json.at("ipAddresses");
    dto.ipAddresses.clear();
    dto.ipAddresses.reserve(ipAddresses.size());

    for (const nlohmann::json& item : ipAddresses) {
        IpAddressDto ipAddress {};
        fromJson(item, ipAddress);
        dto.ipAddresses.push_back(std::move(ipAddress));
    }
}

nlohmann::json toJson(const TerminateIpDto& dto) {
    nlohmann::json ipAddresses = nlohmann::json::array();
    for (const IpAddressDto& ipAddress : dto.ipAddresses) {
        ipAddresses.push_back(toJson(ipAddress));
    }

    return nlohmann::json {
        {"serviceId", dto.serviceId},
        {"ipAddresses", std::move(ipAddresses)},
    };
}

void fromJson(const nlohmann::json& json, TerminateIpDto& dto) {
    json.at("serviceId").get_to(dto.serviceId);
    const nlohmann::json& ipAddresses = json.at("ipAddresses");
    dto.ipAddresses.clear();
    dto.ipAddresses.reserve(ipAddresses.size());

    for (const nlohmann::json& item : ipAddresses) {
        IpAddressDto ipAddress {};
        fromJson(item, ipAddress);
        dto.ipAddresses.push_back(std::move(ipAddress));
    }
}

nlohmann::json toJson(const ChangeServiceDto& dto) {
    return nlohmann::json {
        {"serviceIdOld", dto.serviceIdOld},
        {"serviceId", dto.serviceId},
    };
}

void fromJson(const nlohmann::json& json, ChangeServiceDto& dto) {
    json.at("serviceIdOld").get_to(dto.serviceIdOld);
    json.at("serviceId").get_to(dto.serviceId);
}

} // namespace ip_inv
