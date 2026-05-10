if(NOT DEFINED cc)
    message(FATAL_ERROR "cc is required")
endif()

if(NOT DEFINED cxx)
    message(FATAL_ERROR "cxx is required")
endif()

if(NOT DEFINED cmake_preset)
    message(FATAL_ERROR "cmake_preset is required")
endif()

set(ENV{CC} "${cc}")
set(ENV{CXX} "${cxx}")

# Keep the checked-out repository on PATH for compatibility with workflow helpers.
if(DEFINED ENV{GITHUB_WORKSPACE})
    set(path_separator ":")
    if(DEFINED runner_os AND runner_os STREQUAL "Windows")
        set(path_separator ";")
    endif()

    set(ENV{PATH} "$ENV{GITHUB_WORKSPACE}${path_separator}$ENV{PATH}")
endif()

execute_process(
    COMMAND cmake
        -S .
        -B build
        --preset "${cmake_preset}"
    RESULT_VARIABLE result
)

if(NOT result EQUAL 0)
    message(FATAL_ERROR "CMake configure failed")
endif()
