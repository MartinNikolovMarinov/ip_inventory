#include "profiling.h"

#include <chrono>
#include <iostream>
#include <iomanip>

namespace ip_inv {

namespace {

std::string formatDuration(std::chrono::nanoseconds ns);

} // namespace

ScopeProfiler::ScopeProfiler(std::string_view name)
    : m_name(name)
    , m_wallClock(std::chrono::system_clock::now())
    , m_start(std::chrono::steady_clock::now()) {}

ScopeProfiler::~ScopeProfiler() {
    using namespace std::chrono;

    auto end = steady_clock::now();
    auto elapsed = end - m_start;
    std::time_t timestamp = system_clock::to_time_t(m_wallClock);
    nanoseconds elapsedNs = duration_cast<nanoseconds>(elapsed);

    std::cout
        << "[PROFILE]"
        << " | scope=" << m_name
        << " | called_at="
        << std::put_time(std::localtime(&timestamp), "%Y-%m-%d %H:%M:%S")
        << " | duration="
        << formatDuration(elapsedNs)
        << std::endl;
}

namespace {

std::string formatDuration(std::chrono::nanoseconds ns) {
    const auto count = ns.count();

    if (count < 1'000) {
        return std::format("{}ns", count);
    }

    if (count < 1'000'000) {
        return std::format("{:.2f}us", count / 1'000.0);
    }

    if (count < 1'000'000'000) {
        return std::format("{:.2f}ms", count / 1'000'000.0);
    }

    return std::format("{:.2f}s", count / 1'000'000'000.0);
}

} // namespace

} // namespace ip_inv
