set(build_args "")

if(DEFINED cmake_build_config AND NOT cmake_build_config STREQUAL "")
    separate_arguments(build_args NATIVE_COMMAND "${cmake_build_config}")
endif()

message(STATUS "Build arguments: ${build_args}")

execute_process(
    COMMAND cmake --build build ${build_args} --parallel
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE output
    ECHO_OUTPUT_VARIABLE
    ECHO_ERROR_VARIABLE
)

if(NOT result EQUAL 0)
    string(REGEX MATCH "FAILED:.*$" error_message "${output}")
    string(REPLACE "\n" "%0A" error_message "${error_message}")
    message("::error::${error_message}")
    message(FATAL_ERROR "Build failed")
endif()
