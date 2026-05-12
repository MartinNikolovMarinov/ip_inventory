#include "profiling.h"

#include <iostream>
#include <iomanip>

namespace ip_inv {

ScopeTimer::ScopeTimer(std::string_view name)
    : m_name(name)
    , m_wallClock(std::chrono::system_clock::now())
    , m_start(std::chrono::steady_clock::now()) {}

ScopeTimer::~ScopeTimer() {
    const auto end = std::chrono::steady_clock::now();
    const auto elapsedMs =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - m_start);

    const std::time_t timestamp =
        std::chrono::system_clock::to_time_t(m_wallClock);

    std::cout
        << "[PROFILE]"
        << " | scope=" << m_name
        << " | called_at="
        << std::put_time(std::localtime(&timestamp), "%Y-%m-%d %H:%M:%S")
        << " | duration=" << elapsedMs.count() << "ms"
        << std::endl;
}

} // namespace ip_inv
