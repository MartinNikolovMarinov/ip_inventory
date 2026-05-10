# The following code is adapted from or inspired by the article:
# "Using GitHub Actions with C++ and CMake" by Cristian Adam.
# Original article available at: https://cristianadam.eu/20191222/using-github-actions-with-c-plus-plus-and-cmake/
#
# Credit and appreciation goes to the original author, Cristian Adam, for sharing the knowledge.

# This script is run from the GitHub Actions workflow with:
#   cmake -Dcmake_version=... -Drunner_os=... -P install-cmake.cmake
if(NOT DEFINED cmake_version)
    message(FATAL_ERROR "cmake_version is required")
endif()

if(NOT DEFINED runner_os)
    message(FATAL_ERROR "runner_os is required")
endif()

message(STATUS "Using host CMake version: ${CMAKE_VERSION}")
message(STATUS "Installing CMake version: ${cmake_version}")

# Match GitHub Actions runner names to Kitware's release archive naming.
if(runner_os STREQUAL "Windows")
    set(cmake_suffix "windows-x86_64.zip")
    set(cmake_dir "cmake-${cmake_version}-windows-x86_64/bin")
elseif(runner_os STREQUAL "Linux")
    set(cmake_suffix "Linux-x86_64.tar.gz")
    set(cmake_dir "cmake-${cmake_version}-linux-x86_64/bin")
elseif(runner_os STREQUAL "macOS")
    set(cmake_suffix "macos-universal.tar.gz")
    set(cmake_dir "cmake-${cmake_version}-macos-universal/CMake.app/Contents/bin")
else()
    message(FATAL_ERROR "Unsupported runner OS: ${runner_os}")
endif()

set(cmake_url "https://github.com/Kitware/CMake/releases/download/v${cmake_version}/cmake-${cmake_version}-${cmake_suffix}")
set(cmake_archive "$ENV{GITHUB_WORKSPACE}/cmake-${cmake_version}.${cmake_suffix}")

# Download and unpack into the workspace so the directory layout is predictable.
file(DOWNLOAD "${cmake_url}" "${cmake_archive}" SHOW_PROGRESS)

execute_process(
    COMMAND "${CMAKE_COMMAND}" -E tar xvf "${cmake_archive}"
    WORKING_DIRECTORY "$ENV{GITHUB_WORKSPACE}"
    RESULT_VARIABLE result
)

if(NOT result EQUAL 0)
    message(FATAL_ERROR "Failed to extract CMake")
endif()

file(TO_CMAKE_PATH "$ENV{GITHUB_WORKSPACE}/${cmake_dir}" cmake_dir)

# Add the downloaded CMake to PATH for later workflow steps.
if(DEFINED ENV{GITHUB_PATH})
    file(APPEND "$ENV{GITHUB_PATH}" "${cmake_dir}\n")
endif()

# Expose the directory as a step output in case the workflow needs it later.
if(DEFINED ENV{GITHUB_OUTPUT})
    file(APPEND "$ENV{GITHUB_OUTPUT}" "cmake_dir=${cmake_dir}\n")
endif()

# Linux/macOS archives should already contain executable files, but make them explicitly executable just in case.
if(NOT runner_os STREQUAL "Windows")
    execute_process(
        COMMAND chmod +x "${cmake_dir}/cmake"
        RESULT_VARIABLE result
    )

    if(NOT result EQUAL 0)
        message(FATAL_ERROR "Failed to chmod downloaded cmake")
    endif()
endif()
