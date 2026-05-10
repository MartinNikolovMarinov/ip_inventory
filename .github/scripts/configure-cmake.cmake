if(NOT DEFINED cc)
    message(FATAL_ERROR "cc is required")
endif()

if(NOT DEFINED cxx)
    message(FATAL_ERROR "cxx is required")
endif()

if(NOT DEFINED cmake_preset)
    message(FATAL_ERROR "cmake_preset is required")
endif()

include("${CMAKE_CURRENT_LIST_DIR}/load-environment-script.cmake")
load_environment_script()

set(ENV{CC} "${cc}")
set(ENV{CXX} "${cxx}")

set(configure_args "")
if(DEFINED cmake_config AND NOT cmake_config STREQUAL "")
    separate_arguments(configure_args NATIVE_COMMAND "${cmake_config}")
endif()

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
        ${configure_args}
    RESULT_VARIABLE result
)

if(NOT result EQUAL 0)
    message(FATAL_ERROR "CMake configure failed")
endif()
