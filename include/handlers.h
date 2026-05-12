#pragma once

#include "inventory/service.h"

#include <httplib.h>

namespace ip_inv {

void addIpPoolHandler(IpInventoryService& inventoryService, const httplib::Request& req, httplib::Response& res);

void serveFile(const char* path, const char* contentType, httplib::Response& response);

} // namespace ip_inv
