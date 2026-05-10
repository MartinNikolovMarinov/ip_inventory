cmake_minimum_required(VERSION 3.20)
project(system_programming VERSION 0.0.1)

set(ip_inventory_target ip_inventory)

# Standard Requirements:

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # make sure no gnu/clang specific language extensions are used
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # Generates compile_commands.json

# Include cmake modules from git submodules:

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(UpdateGitSubmodules)
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

target_set_default_c_flags(ip_inventory ${IP_INVENTORY_DEBUG})

target_link_libraries(ip_inventory PRIVATE Drogon::Drogon)

# ---------------------------------------- End Create Executables ------------------------------------------------------

# ---------------------------------------- Begin Tests ----------------------------------------------------------------

enable_testing()

add_library(unity STATIC tests/unity/unity.c)
target_include_directories(unity PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unity
)

function(add_unity_test test_name test_source)
    add_executable(${test_name} ${test_source})
    target_include_directories(${test_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(${test_name} PRIVATE unity)
    target_set_default_c_flags(${test_name} ${IP_INVENTORY_DEBUG})
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${test_name} PRIVATE
            -Wno-old-style-cast
            -Wno-pragmas
        )
    endif()
    add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

add_unity_test(ip_inventory_test_types tests/test_types.cpp)
add_unity_test(ip_inventory_test_constants tests/test_constants.cpp)

# ---------------------------------------- End Tests ------------------------------------------------------------------
