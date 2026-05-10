function(target_set_default_c_flags target is_debug)

    target_compile_options(${target} PRIVATE
        /MP                                 # Build multiple source files in parallel
        /W4                                 # Enable high warning level without the excessive noise of /Wall
        /nologo                             # Suppress the MSVC compiler startup banner
        /FC                                 # Print full paths in diagnostics
        /utf-8                              # Treat source and execution character sets as UTF-8
        /permissive-                        # Specify standards conformance mode to the compiler.
        /fastfail                           # Enable fast-fail behavior where supported
        /Zc:__cplusplus                     # Make __cplusplus report the real selected C++ standard instead of MSVC's historical stale value
        /Zc:preprocessor                    # Use the conforming C++ preprocessor
        /Zc:externConstexpr                 # Apply standard-conforming linkage rules for extern constexpr variables
        /EHsc                               # Enable standard C++ exception handling and assume extern "C" functions do not throw
        /external:W0                        # Suppress warnings from headers classified as external

        /diagnostics:caret                  # Show caret diagnostics with source-line context

        /w14254                             # Warn when more than one user-defined conversion is needed for an implicit conversion
        /w14263                             # Warn when a member function hides a base class virtual function
        /w14265                             # Warn when a class has virtual functions but a non-virtual destructor
        /w14287                             # Warn when unsigned values are used in conditional expressions in suspicious ways
        /w14296                             # Warn when an expression is always true or always false
        /w14311                             # Warn when a pointer truncation may occur
        /w14242                             # Warn when a conversion may lose data
        /w14545                             # Warn when an expression before a comma evaluates to a function
        /w14546                             # Warn when an expression before a comma evaluates to a function with no arguments
        /w14547                             # Warn when an operator before a comma has no effect
        /w14549                             # Warn when an operator before a comma has no effect and may indicate a logic error
        /w14555                             # Warn when an expression has no effect
        /w14640                             # Warn when thread-safe initialization of local statics is not guaranteed
    )

    if(is_debug)
        target_compile_options(${target} PRIVATE
            /Zi                             # Generate complete debugging information in a PDB
            /Od                             # Disable optimizations for easier debugging
            /RTC1                           # Enable basic runtime checks for stack frames, uninitialized variables, and truncation
            /JMC                            # Enable Just My Code debugging support
        )
    else()
        target_compile_options(${target} PRIVATE
            /O2                             # Enable high-speed optimizations
        )
    endif()

endfunction()
