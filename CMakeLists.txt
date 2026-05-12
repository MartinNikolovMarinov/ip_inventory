cmake_minimum_required(VERSION 3.20)
project(system_programming VERSION 0.0.1)

set(ip_inventory_target ip_inventory)

# Standard Requirements:

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # make sure no gnu/clang specific language extensions are used
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # Generates compile_commands.json

# Include cmake helper modules:

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
option(IP_INVENTORY_BUILD_TESTS "Build tests." OFF)
option(IP_INVENTORY_CONFIGURED_FROM_PRESET "Configured from preset." OFF)
set(IP_INVENTORY_SQLITE3_DB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/db" CACHE PATH "Directory for the SQLite database.")

log_info("---------------------------------------------")
log_info("Version:                   ${PROJECT_VERSION}")
log_info("Platform:                  ${OS}")
log_info("Arch:                      ${CMAKE_SYSTEM_PROCESSOR}")
log_info("C++ Version:               ${CMAKE_CXX_STANDARD}")
log_info("Compiler:                  ${CMAKE_CXX_COMPILER_ID}")
log_info("Compiler Version:          ${CMAKE_CXX_COMPILER_VERSION}")
log_info("Configured from preset:    ${IP_INVENTORY_CONFIGURED_FROM_PRESET}")
log_info("Debug:                     ${IP_INVENTORY_DEBUG}")
log_info("Build Tests:               ${IP_INVENTORY_BUILD_TESTS}")
log_info("Assert Enabled:            ${IP_INVENTORY_ASSERT_ENABLED}")
log_info("SQLite3 DB Root:           ${IP_INVENTORY_SQLITE3_DB_ROOT}")

# ---------------------------------------- End Options -----------------------------------------------------------------

# ---------------------------------------- Begin Build Dependencies ----------------------------------------------------

add_subdirectory(vendor/sqlite)
add_subdirectory(vendor/nlohmann)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(HTTPLIB_INSTALL OFF CACHE BOOL "" FORCE)
set(HTTPLIB_TEST OFF CACHE BOOL "" FORCE)
add_subdirectory(vendor/cpp-httplib)

# Mark the produced libraries as system libraries to silence build warnings for them.
set_property(TARGET sqlite3 PROPERTY SYSTEM ON)
set_property(TARGET httplib PROPERTY SYSTEM ON)

# ---------------------------------------- End Build Dependencies ------------------------------------------------------

# ---------------------------------------- Begin Declare Source Files --------------------------------------------------

set(ip_inventory_src
    main.cpp

    src/app.cpp
    src/handlers.cpp
    src/ip_utils.cpp
    src/validation.cpp
    src/dtos.cpp
    src/profiling.cpp

    src/sqlite/sqlite.cpp

    src/inventory/service.cpp
    src/inventory/sqllite3_repository.cpp
)

set(ip_inventory_test_ip_utils_src
    tests/test_ip_utils.cpp
    src/ip_utils.cpp
)

set(ip_inventory_test_validation_src
    tests/test_validation.cpp
    src/validation.cpp
)

# ---------------------------------------- End Declare Source Files ----------------------------------------------------

# ---------------------------------------- Begin Create Executables ----------------------------------------------------

add_executable(${ip_inventory_target} ${ip_inventory_src})
target_include_directories(ip_inventory PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_definitions(ip_inventory PRIVATE
    IP_INVENTORY_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}" # TODO: The ip inventory directory is used for service static files, it should instead use the binary directory and copy dist there!! Same for schema sql.
    IP_INVENTORY_SQLITE3_DB_ROOT="${IP_INVENTORY_SQLITE3_DB_ROOT}"
)

target_set_default_c_flags(ip_inventory ${IP_INVENTORY_DEBUG})

target_link_libraries(ip_inventory PRIVATE
    sqlite3
    httplib::httplib
    nlohmann_json::nlohmann_json
)

if(OS STREQUAL "windows")
    # inet_pton is provided by Winsock on Windows, so targets using ip_utils.cpp must link ws2_32.
    target_link_libraries(ip_inventory PRIVATE ws2_32)
endif()

# ---------------------------------------- End Create Executables ------------------------------------------------------

# ---------------------------------------- Begin Tests -----------------------------------------------------------------

if (IP_INVENTORY_BUILD_TESTS)

enable_testing()

add_library(unity STATIC tests/unity/unity.c)
target_include_directories(unity PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unity
)
set_property(TARGET unity PROPERTY SYSTEM ON)

function(add_unity_test test_name test_source)
    add_executable(${test_name} ${test_source})

    target_include_directories(${test_name} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    target_compile_definitions(${test_name} PRIVATE
        IP_INVENTORY_TEST_DATA_ROOT="${CMAKE_BINARY_DIR}/test_data"
    )

    target_link_libraries(${test_name} PRIVATE unity ${ARGN})
    target_set_default_c_flags(${test_name} ${IP_INVENTORY_DEBUG})

    add_test(NAME ${test_name} COMMAND ${test_name})

endfunction()

add_unity_test(ip_inventory_test_validation "${ip_inventory_test_validation_src}")
add_unity_test(ip_inventory_test_ip_utils "${ip_inventory_test_ip_utils_src}")

if(OS STREQUAL "windows")
    # inet_pton is provided by Winsock on Windows, so targets using ip_utils.cpp must link ws2_32.
    target_link_libraries(ip_inventory_test_ip_utils PRIVATE ws2_32)
endif()

endif()

# ---------------------------------------- End Tests ------------------------------------------------------------------
