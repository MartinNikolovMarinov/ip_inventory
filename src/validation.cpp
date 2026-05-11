#include "validation.h"
#include "types.h"

namespace ip_inv {

bool isValidPort(i32 port) {
    bool ret = port > 0 && port < 65535;
    return ret;
}

} // namespace ip_inv
