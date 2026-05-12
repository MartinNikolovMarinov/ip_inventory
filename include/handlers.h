#pragma once

#include "inventory/service.h"

#include <httplib.h>

namespace ip_inv {

void addIpPoolHandler(IpInventoryService& inventoryService, const httplib::Request& req, httplib::Response& res);

} // namespace ip_inv
