set(SQLite3_FOUND TRUE)
set(SQLITE3_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/vendor/sqlite")
set(SQLITE3_LIBRARIES sqlite3)

if(NOT TARGET SQLite3_lib)
    # Create an imported interface target that wraps the vendored sqlite3 library.
    add_library(SQLite3_lib INTERFACE IMPORTED)
    set_target_properties(SQLite3_lib PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${SQLITE3_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES sqlite3
    )
endif()

# hides cache variables from normal CMake GUIs (or ccmake):
mark_as_advanced(SQLITE3_INCLUDE_DIRS SQLITE3_LIBRARIES)
