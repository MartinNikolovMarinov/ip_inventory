#pragma once

#include "types.h"

#include <string>

struct sqlite3;
struct sqlite3_stmt;

namespace ip_inv {

[[nodiscard]] const char* sqliteError(sqlite3* db) noexcept;
void assertSqliteOk(i32 result, sqlite3* db, const char* operation);
void assertSqliteDone(i32 result, sqlite3* db, const char* operation);

class SqliteTransaction {
public:
    explicit SqliteTransaction(sqlite3* db);
    ~SqliteTransaction() noexcept;

    SqliteTransaction(const SqliteTransaction&) = delete;
    SqliteTransaction& operator=(const SqliteTransaction&) = delete;
    SqliteTransaction(SqliteTransaction&& other) = delete;
    SqliteTransaction& operator=(SqliteTransaction&& other) = delete;

    void commit();
    void rollback() noexcept;

private:
    sqlite3* m_db = nullptr;
    bool m_finished = true;
};

class SqliteStatement {
public:
    SqliteStatement(sqlite3* db, const char* sql);
    ~SqliteStatement() noexcept;

    SqliteStatement(const SqliteStatement&) = delete;
    SqliteStatement& operator=(const SqliteStatement&) = delete;
    SqliteStatement(SqliteStatement&& other) = delete;
    SqliteStatement& operator=(SqliteStatement&& other) = delete;

    void bindInt(i32 index, i32 value);
    void bindInt64(i32 index, i64 value);
    void bindText(i32 index, const std::string& value);
    void bindBlob(i32 index, const void* data, i32 size);
    void bindNull(i32 index);

    [[nodiscard]] bool step();
    void reset();
    void clearBindings();

private:
    sqlite3* m_db = nullptr;
    sqlite3_stmt* m_statement = nullptr;
};

} // namespace ip_inv
