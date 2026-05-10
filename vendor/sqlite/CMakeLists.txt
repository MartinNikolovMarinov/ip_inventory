add_library(sqlite3 STATIC sqlite3.c)
add_library(SQLite::SQLite3 ALIAS sqlite3)

target_include_directories(sqlite3 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(sqlite3 PUBLIC
    SQLITE_THREADSAFE=1
)

target_link_libraries(sqlite3 PUBLIC
    ${CMAKE_DL_LIBS}
)

set_property(TARGET sqlite3 PROPERTY SYSTEM ON)
