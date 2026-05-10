include("${CMAKE_CURRENT_LIST_DIR}/load-environment-script.cmake")
load_environment_script()

set(ENV{CTEST_OUTPUT_ON_FAILURE} "ON")

set(test_args "")

if(DEFINED ctest_config AND NOT ctest_config STREQUAL "")
    separate_arguments(test_args NATIVE_COMMAND "${ctest_config}")
endif()

message(STATUS "CTest arguments: ${test_args}")

# Keep tests serial unless ctest_config explicitly includes parallelism.
execute_process(
    COMMAND ctest ${test_args} --verbose
    WORKING_DIRECTORY build
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE output
    ECHO_OUTPUT_VARIABLE
    ECHO_ERROR_VARIABLE
)

if(NOT result EQUAL 0)
    string(REGEX MATCH "[0-9]+% tests.*[0-9.]+ sec.*$" test_results "${output}")
    string(REPLACE "\n" "%0A" test_results "${test_results}")
    message("::error::${test_results}")
    message(FATAL_ERROR "Running tests failed")
endif()
