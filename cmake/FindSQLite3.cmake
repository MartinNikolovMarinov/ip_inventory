if(TARGET sqlite3)
    set(SQLite3_FOUND TRUE)
    set(SQLITE3_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/vendor/sqlite")
    set(SQLITE3_LIBRARIES sqlite3)

    if(NOT TARGET SQLite3_lib)
        add_library(SQLite3_lib INTERFACE IMPORTED)
        set_target_properties(SQLite3_lib PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${SQLITE3_INCLUDE_DIRS}"
            INTERFACE_LINK_LIBRARIES sqlite3
        )
    endif()
else()
    find_path(SQLITE3_INCLUDE_DIRS NAMES sqlite3.h)
    find_library(SQLITE3_LIBRARIES NAMES sqlite3)

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(SQLite3
        DEFAULT_MSG
        SQLITE3_LIBRARIES
        SQLITE3_INCLUDE_DIRS
    )

    if(SQLite3_FOUND AND NOT TARGET SQLite3_lib)
        add_library(SQLite3_lib INTERFACE IMPORTED)
        set_target_properties(SQLite3_lib PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${SQLITE3_INCLUDE_DIRS}"
            INTERFACE_LINK_LIBRARIES "${SQLITE3_LIBRARIES}"
        )
    endif()
endif()

mark_as_advanced(SQLITE3_INCLUDE_DIRS SQLITE3_LIBRARIES)
