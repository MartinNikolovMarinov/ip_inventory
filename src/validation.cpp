#include "validation.h"
#include "types.h"

namespace ip_inv {

bool isValidDatabaseName(const std::string& name) {
    if (name.empty() || name == "." || name == ".." || name.length() > 128) {
        return false;
    }

    for (char ch : name) {
        const bool isLetter = (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
        const bool isDigit = ch >= '0' && ch <= '9';
        const bool isAllowedSymbol = ch == '_' || ch == '-' || ch == '.';
        if (!isLetter && !isDigit && !isAllowedSymbol) {
            return false;
        }
    }

    return true;
}

bool isValidPort(i32 port) {
    bool ret = port > 0 && port < 65535;
    return ret;
}

} // namespace ip_inv
