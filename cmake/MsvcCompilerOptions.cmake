function(ip_inventory_target_set_default_c_flags target is_debug)
    target_compile_options(${target} PRIVATE
        /MP
        -Wall -W4
        -nologo -FC
        /fastfail

        /w14254 # Warns if more than one operator conversion is applied to get from one type to another (useful for finding implicit conversions that might be unintended).
        /w14263 # Warns if a member function hides a base class virtual function (similar to -Woverloaded-virtual in GCC/Clang).
        /w14265 # Warns if a class is derived from two or more classes that are not interfaces.
        /w14287 # Warns for 'unsigned' char, short, int or long types in a conditional expression, which can lead to unexpected behaviors.
        /w14296 # Warns if a constructor is declared without specifying either 'explicit' or 'implicit'.
        /w14311 # Warns if a catch block has been written to catch C++ exceptions by value rather than by reference.
    )

    if(is_debug)
        target_compile_options(${target} PRIVATE
            /Zi
            /Od
            /RTC1
        )
    else()
        target_compile_options(${target} PRIVATE
            /O2
        )
    endif()
endfunction()
