#pragma once

#include <string_view>
#include <chrono>

namespace ip_inv {

class ScopeTimer {
public:
    explicit ScopeTimer(std::string_view name);
    ~ScopeTimer();

private:
    std::string_view m_name;
    std::chrono::system_clock::time_point m_wallClock;
    std::chrono::steady_clock::time_point m_start;
};

} // namespace ip_inv
