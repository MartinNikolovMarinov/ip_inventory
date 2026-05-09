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

set(BUILD_CTL OFF CACHE BOOL "Build drogon_ctl" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "Build Drogon examples" FORCE)
set(BUILD_ORM OFF CACHE BOOL "Build Drogon ORM" FORCE)
set(BUILD_BROTLI OFF CACHE BOOL "Build Drogon Brotli support" FORCE)
set(BUILD_YAML_CONFIG OFF CACHE BOOL "Build Drogon YAML config support" FORCE)

add_subdirectory(vendor/drogon)

# Include cmake modules from git submodules:

# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# include(DetectOS)
# include(CompilerOptions)
# include(Logger)

# init_logger("[SYSTEM_PROGRAMMING]")

# ---------------------------------------- Begin Options ---------------------------------------------------------------

# Print Selected Options:

# option(USE_ANSI_LOGGING "Use ANSI logging." OFF)

# log_info("---------------------------------------------")
# log_info("Version:                   ${PROJECT_VERSION}")
# log_info("Platform:                  ${OS}")
# log_info("Arch:                      ${CMAKE_SYSTEM_PROCESSOR}")
# log_info("C++ Version:               ${CMAKE_CXX_STANDARD}")
# log_info("Compiler:                  ${CMAKE_CXX_COMPILER_ID}")
# log_info("Compiler Version:          ${CMAKE_CXX_COMPILER_VERSION}")
# log_info("Debug:                     ${SP_DEBUG}")
# log_info("Use Ansi Logging:          ${USE_ANSI_LOGGING}")

# ---------------------------------------- End Options -----------------------------------------------------------------

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

target_link_libraries(ip_inventory PRIVATE Drogon::Drogon)

# ---------------------------------------- End Create Executables ------------------------------------------------------
