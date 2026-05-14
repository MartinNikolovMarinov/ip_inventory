#include "sqlite/sqlite.h"

#include <sqlite3.h>

#include <format>
#include <stdexcept>

namespace ip_inv {

const char* sqliteError(sqlite3* db) noexcept {
    return db == nullptr ? "unknown sqlite error" : sqlite3_errmsg(db);
}

void assertSqliteOk(i32 result, sqlite3* db, const char* operation) {
    if (result == SQLITE_OK) {
        return;
    }

    throw std::runtime_error(std::format("{}: {}", operation, sqliteError(db)));
}

void assertSqliteDone(i32 result, sqlite3* db, const char* operation) {
    if (result == SQLITE_DONE) {
        return;
    }

    throw std::runtime_error(std::format("{}: {}", operation, sqliteError(db)));
}

SqliteTransaction::SqliteTransaction(sqlite3* db)
    : m_db(db),
      m_finished(false) {
    if (m_db == nullptr) {
        throw std::invalid_argument("sqlite transaction requires an open database");
    }

    assertSqliteOk(sqlite3_exec(m_db, "BEGIN;", nullptr, nullptr, nullptr), m_db, "begin transaction");
}

SqliteTransaction::~SqliteTransaction() noexcept {
    rollback();
}

void SqliteTransaction::commit() {
    if (m_finished) {
        return;
    }

    assertSqliteOk(sqlite3_exec(m_db, "COMMIT;", nullptr, nullptr, nullptr), m_db, "commit transaction");
    m_finished = true;
}

void SqliteTransaction::rollback() noexcept {
    if (!m_finished && m_db != nullptr) {
        sqlite3_exec(m_db, "ROLLBACK;", nullptr, nullptr, nullptr);
        m_finished = true;
    }
}

SqliteStatement::SqliteStatement(sqlite3* db, const char* sql)
    : m_db(db) {
    if (m_db == nullptr) {
        throw std::invalid_argument("sqlite statement requires an open database");
    }

    assertSqliteOk(
        sqlite3_prepare_v2(m_db, sql, -1, &m_statement, nullptr),
        m_db,
        "prepare statement"
    );
}

SqliteStatement::~SqliteStatement() noexcept {
    if (m_statement != nullptr) {
        sqlite3_finalize(m_statement);
        m_statement = nullptr;
    }
}

void SqliteStatement::bindInt(i32 index, i32 value) {
    assertSqliteOk(sqlite3_bind_int(m_statement, index, value), m_db, "bind int");
}

void SqliteStatement::bindInt64(i32 index, i64 value) {
    assertSqliteOk(sqlite3_bind_int64(m_statement, index, value), m_db, "bind int64");
}

void SqliteStatement::bindText(i32 index, const std::string& value) {
    assertSqliteOk(
        sqlite3_bind_text(m_statement, index, value.c_str(), -1, SQLITE_TRANSIENT),
        m_db,
        "bind text"
    );
}

void SqliteStatement::bindBlob(i32 index, const void* data, i32 size) {
    assertSqliteOk(sqlite3_bind_blob(m_statement, index, data, size, SQLITE_TRANSIENT), m_db, "bind blob");
}

void SqliteStatement::bindNull(i32 index) {
    assertSqliteOk(sqlite3_bind_null(m_statement, index), m_db, "bind null");
}

i32 SqliteStatement::columnInt(i32 index) const {
    return sqlite3_column_int(m_statement, index);
}

i64 SqliteStatement::columnInt64(i32 index) const {
    return sqlite3_column_int64(m_statement, index);
}

const void* SqliteStatement::columnBlob(i32 index) const {
    return sqlite3_column_blob(m_statement, index);
}

i32 SqliteStatement::columnBytes(i32 index) const {
    return sqlite3_column_bytes(m_statement, index);
}

std::string SqliteStatement::columnText(i32 index) const {
    const unsigned char* text = sqlite3_column_text(m_statement, index);
    return text == nullptr ? std::string {} : reinterpret_cast<const char*>(text);
}

bool SqliteStatement::columnIsNull(i32 index) const {
    return sqlite3_column_type(m_statement, index) == SQLITE_NULL;
}

void SqliteStatement::execute() {
    assertSqliteDone(sqlite3_step(m_statement), m_db, "execute statement");
}

bool SqliteStatement::stepRow() {
    const i32 result = sqlite3_step(m_statement);
    if (result == SQLITE_ROW) {
        return true;
    }

    assertSqliteDone(result, m_db, "execute statement");
    return false;
}

void SqliteStatement::reset() {
    assertSqliteOk(sqlite3_reset(m_statement), m_db, "reset statement");
}

void SqliteStatement::clearBindings() {
    assertSqliteOk(sqlite3_clear_bindings(m_statement), m_db, "clear statement bindings");
}

} // namespace ip_inv
