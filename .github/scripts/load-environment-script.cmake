function(load_environment_script)
    if(NOT DEFINED environment_script OR "x${environment_script}" STREQUAL "x")
        return()
    endif()

    if(NOT EXISTS "${environment_script}")
        message(FATAL_ERROR "Environment script does not exist: ${environment_script}")
    endif()

    # Import variables produced by a setup script into this CMake process.
    # This is mainly needed for Windows MSVC activation via vcvars64.bat,
    # which provides PATH, INCLUDE, LIB, and other compiler/linker settings.
    execute_process(
        COMMAND "${environment_script}" && set
        OUTPUT_FILE environment_script_output.txt
        RESULT_VARIABLE result
    )

    if(NOT result EQUAL 0)
        message(FATAL_ERROR "Failed to run environment script: ${environment_script}")
    endif()

    file(STRINGS environment_script_output.txt output_lines)
    foreach(line IN LISTS output_lines)
        if(line MATCHES "^([a-zA-Z0-9_-]+)=(.*)$")
            set(ENV{${CMAKE_MATCH_1}} "${CMAKE_MATCH_2}")
        endif()
    endforeach()
endfunction()
