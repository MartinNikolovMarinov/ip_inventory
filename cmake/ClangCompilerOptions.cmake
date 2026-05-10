function(target_set_default_c_flags target is_debug)

    # Rewrite absolute source paths in debug info/macros to relative paths to avoid leaking build machine details.
    target_compile_options(${target} PRIVATE -ffile-prefix-map=${CMAKE_SOURCE_DIR}=.)

    if (CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)")
        # Use Intel assembly syntax instead of AT&T syntax in generated assembly output
        target_compile_options(${target} PRIVATE
            -masm=intel
        )
    endif()

    target_compile_options(${target} PRIVATE
        -Wall                                     # Enable common warning set
        -Wextra                                   # Enable extra useful warnings not included in -Wall
        -Wfatal-errors                            # Stop after the first error
        -Wpedantic                                # Enforce strict ISO C++ compliance warnings

        -Wshadow                                  # Warn when a local declaration shadows another declaration
        -Wshadow-all                              # Warn about all declaration shadowing cases, including fields and compatible local shadowing
        -Wold-style-cast                          # Warn on usage of old-style C casts
        -Wdouble-promotion                        # Warn when float values are implicitly promoted to double
        -Wswitch-enum                             # Warn when switch statements do not handle all enum values
        -Wimplicit-fallthrough=5                  # Require explicit annotation for switch fallthroughs
        -Wduplicate-enum                          # Warn about duplicate values in enum declarations
        -Wundef                                   # Warn if an undefined macro is evaluated in #if expressions
        -Wcast-align                              # Warn about casts that increase required memory alignment
        -Wnull-dereference                        # Warn about potential null pointer dereferences detected by analysis
        -Wextra-semi                              # Warn about unnecessary semicolons
        -Wmisleading-indentation                  # Warn when indentation suggests misleading control flow
        -Woverloaded-virtual                      # Warn when virtual overloads are hidden by derived declarations
        -Wnon-virtual-dtor                        # Warn when a polymorphic class lacks a virtual destructor
        -Wsuggest-override                        # Warn when override could be used but is missing
        -Winconsistent-missing-override           # Warn when some overriding methods use override but others do not
        -Wformat=2                                # Enable stricter printf/scanf format checking
        -Wduplicated-cond                         # Warn about duplicated conditions in if/else chains
        -Wduplicated-branches                     # Warn when both branches of a conditional are identical
        -Wlogical-op                              # Warn about suspicious logical operations
        -Wzero-as-null-pointer-constant           # Warn when literal 0 is used instead of nullptr
        -Wmissing-declarations                    # Warn about global functions without prior declarations
        -Wredundant-decls                         # Warn about redundant repeated declarations
        -Wcast-qual                               # Warn when casts discard const/volatile qualifiers
        -Wwrite-strings                           # Treat string literals as const char[] and warn on unsafe conversions
        -Wstring-conversion                       # Warn about implicit conversions from string literals that change meaning or type safety
        -Wconditional-uninitialized               # Warn when a variable may be used uninitialized along some conditional path
        -Wextra-semi-stmt                         # Warn about redundant semicolons after statements
        -Wrange-loop-analysis                     # Warn about inefficient or unintended range-based for loop copies
        -Wreserved-identifier                     # Warn about identifiers reserved to the implementation
        -Wself-move                               # Warn about explicitly moving an object into itself
        -Wthread-safety                           # Enable Clang thread-safety analysis diagnostics for annotated code
        -Wundefined-reinterpret-cast              # Warn about reinterpret_cast cases that produce undefined behavior
        -Wunreachable-code-break                  # Warn about break statements after unreachable code
        -Wunused-exception-parameter              # Warn about unused exception handler parameters

        -Wdisabled-optimization                   # Warn if requested optimizations are disabled internally by GCC

        -Wno-unknown-pragmas                      # Ignore unknown #pragma directives
        -Wno-unused-function                      # Do not warn about unused static/internal functions
        -Wno-variadic-macros                      # Do not warn about variadic macro usage extensions
        -Wno-gnu-zero-variadic-macro-arguments    # Suppress Clang warning for GCC-style , ##__VA_ARGS__ variadic macro extension
        -Wno-c++98-compat                         # Suppress warnings about incompatibility with C++98
        -Wno-c++98-compat-pedantic                # Suppress pedantic warnings about incompatibility with C++98
    )

    if(is_debug)
        target_compile_options(${target} PRIVATE
            -g                          # Generate debug symbols
            -O0                         # Disable optimizations for easier debugging
            -Wno-unused-function        # Suppress unused function warnings in debug builds
            -Wno-unused-variable        # Suppress unused variable warnings in debug builds
        )
    else()
        target_compile_options(${target} PRIVATE
            -O2                         # Enable moderate optimization level

            -Wunused-function           # Warn about unused functions
            -Wunused-variable           # Warn about unused variables

            -fstack-protector-strong    # Enable stronger stack smashing protection
            -D_FORTIFY_SOURCE=3         # Enable fortified libc checks for common unsafe operations
            -fPIE                       # Generate position independent code for PIE executables
        )
    endif()

endfunction()
