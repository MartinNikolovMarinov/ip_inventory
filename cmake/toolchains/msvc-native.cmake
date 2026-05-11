# Native MSVC toolchain for local builds.
# Usage from a Visual Studio Developer shell:
#   cmake --preset msvc-debug
#   cmake --preset msvc-release

set(CMAKE_C_COMPILER cl CACHE STRING "C compiler" FORCE)
set(CMAKE_CXX_COMPILER cl CACHE STRING "C++ compiler" FORCE)
