# Native GCC toolchain for local builds.
# Usage:
#   cmake --preset gcc-debug
#   cmake --preset gcc-release

set(CMAKE_C_COMPILER gcc CACHE STRING "C compiler" FORCE)
set(CMAKE_CXX_COMPILER g++ CACHE STRING "C++ compiler" FORCE)
set(CMAKE_ASM_COMPILER gcc CACHE STRING "ASM compiler" FORCE)

# Prefer GCC-specific binutils wrappers when available.
find_program(GCC_AR NAMES gcc-ar)
if(GCC_AR)
    set(CMAKE_AR "${GCC_AR}" CACHE FILEPATH "Archiver" FORCE)
else()
    message(WARNING "gcc-native toolchain: gcc-ar not found; using CMake default archiver.")
endif()

find_program(GCC_RANLIB NAMES gcc-ranlib)
if(GCC_RANLIB)
    set(CMAKE_RANLIB "${GCC_RANLIB}" CACHE FILEPATH "Ranlib" FORCE)
else()
    message(WARNING "gcc-native toolchain: gcc-ranlib not found; using CMake default ranlib.")
endif()

find_program(GCC_NM NAMES gcc-nm)
if(GCC_NM)
    set(CMAKE_NM "${GCC_NM}" CACHE FILEPATH "nm" FORCE)
else()
    message(WARNING "gcc-native toolchain: gcc-nm not found; using CMake default nm.")
endif()
