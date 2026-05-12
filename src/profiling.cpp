#include "profiling.h"

#include <iostream>
#include <iomanip>

namespace ip_inv {

ScopeProfiler::ScopeProfiler(std::string_view name)
    : m_name(name)
    , m_wallClock(std::chrono::system_clock::now())
    , m_start(std::chrono::steady_clock::now()) {}

ScopeProfiler::~ScopeProfiler() {
    using namespace std::chrono;

    auto end = steady_clock::now();
    auto elapsed = end - m_start;
    std::time_t timestamp = system_clock::to_time_t(m_wallClock);

    std::cout
        << "[PROFILE]"
        << " | scope=" << m_name
        << " | called_at="
        << std::put_time(std::localtime(&timestamp), "%Y-%m-%d %H:%M:%S")
        << " | duration="
        << std::format("{}", duration_cast<nanoseconds>(elapsed))
        << std::endl;
}

} // namespace ip_inv
