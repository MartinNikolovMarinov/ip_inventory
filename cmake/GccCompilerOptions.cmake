function(ip_inventory_target_set_default_c_flags target is_debug)

    target_compile_options(${target} PRIVATE -ffile-prefix-map=${CMAKE_SOURCE_DIR}=.)

    if (CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)")
        target_compile_options(${target} PRIVATE -masm=intel)
    endif()

    target_compile_options(${target} PRIVATE
        -Wall
        -Wextra
        -Wfatal-errors
        -Wconversion
        -Wpedantic

        -Wshadow
        -Wold-style-cast
        -Wdouble-promotion
        -Wswitch-enum
        -Wundef
        -Wcast-align
        -Wextra-semi
        -Wmisleading-indentation
        -Woverloaded-virtual
        -Wnon-virtual-dtor

        -Wdisabled-optimization # warn if the compiler disables requested level of optimization

        -Wno-unknown-pragmas
        -Wno-unused-function
        -Wno-variadic-macros

        -Wno-gnu-zero-variadic-macro-arguments # Suppress warning for " , ##__VA_ARGS__ " in variadic macros
    )

    if(is_debug)
        target_compile_options(${target} PRIVATE
            -g
            -O0
            -Wno-unused-function
            -Wno-unused-variable
        )
    else()
        target_compile_options(${target} PRIVATE
            -O2
            -Wunused-function
            -Wunused-variable
        )
    endif()

endfunction()
