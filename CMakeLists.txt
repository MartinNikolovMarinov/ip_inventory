cmake_minimum_required(VERSION 3.20)
project(system_programming VERSION 0.0.1)

set(ip_inventory_target ip_inventory)

# Download and update git submodules:

find_package(Git QUIET)
if (GIT_FOUND)
    message(STATUS "Submodule update")
    execute_process(
        COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        RESULT_VARIABLE GIT_SUBMOD_RESULT
    )
    if (NOT GIT_SUBMOD_RESULT EQUAL "0")
        message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
    endif()
else()
    message(WARNING "Git is not found. It is required to update submodules.")
endif()

# Standard Requirements:

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # Generates compile_commands.json

# Include cmake modules from git submodules:

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(DetectOS)
include(CompilerOptions)
include(Logger)

init_logger("[IP_INVENTORY]")

# ---------------------------------------- Begin Options ---------------------------------------------------------------

# Print Selected Options:

if(NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "Release")
    set(IP_INVENTORY_DEBUG OFF)
else()
    set(IP_INVENTORY_DEBUG ON)
endif()

option(IP_INVENTORY_ASSERT_ENABLED "Asserts enabled." OFF)

log_info("---------------------------------------------")
log_info("Version:                   ${PROJECT_VERSION}")
log_info("Platform:                  ${OS}")
log_info("Arch:                      ${CMAKE_SYSTEM_PROCESSOR}")
log_info("C++ Version:               ${CMAKE_CXX_STANDARD}")
log_info("Compiler:                  ${CMAKE_CXX_COMPILER_ID}")
log_info("Compiler Version:          ${CMAKE_CXX_COMPILER_VERSION}")
log_info("Debug:                     ${IP_INVENTORY_DEBUG}")
log_info("Assert Enabled:            ${IP_INVENTORY_ASSERT_ENABLED}")

# ---------------------------------------- End Options -----------------------------------------------------------------

# ---------------------------------------- Begin Build Dependencies ----------------------------------------------------

set(BUILD_CTL OFF CACHE BOOL "Build drogon_ctl" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "Build Drogon examples" FORCE)
set(BUILD_ORM OFF CACHE BOOL "Build Drogon ORM" FORCE)
set(BUILD_BROTLI OFF CACHE BOOL "Build Drogon Brotli support" FORCE)
set(BUILD_YAML_CONFIG OFF CACHE BOOL "Build Drogon YAML config support" FORCE)

add_subdirectory(vendor/drogon)

if (TARGET drogon AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(drogon PRIVATE -Wno-deprecated-declarations)
endif()

# ---------------------------------------- End Build Dependencies ------------------------------------------------------

# ---------------------------------------- Begin Declare Source Files --------------------------------------------------

set(ip_inventory_src
    main.cpp
)

# ---------------------------------------- End Declare Source Files ----------------------------------------------------


# ---------------------------------------- Begin Create Executables ----------------------------------------------------

add_executable(${ip_inventory_target} ${ip_inventory_src})
target_include_directories(ip_inventory PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_definitions(ip_inventory PRIVATE
    IP_INVENTORY_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

ip_inventory_target_set_default_c_flags(ip_inventory ${IP_INVENTORY_DEBUG})

target_link_libraries(ip_inventory PRIVATE Drogon::Drogon)

# ---------------------------------------- End Create Executables ------------------------------------------------------
