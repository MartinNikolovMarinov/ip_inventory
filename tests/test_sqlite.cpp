#include <sqlite3.h>
#include <unity.h>

#include <chrono>
#include <filesystem>
#include <string>

static std::filesystem::path testRunDirectory;

void setUp() {}

void tearDown() {
    if (!testRunDirectory.empty()) {
        std::filesystem::remove_all(testRunDirectory);
        testRunDirectory.clear();
    }
}

static std::string test_run_timestamp() {
    const auto now = std::chrono::system_clock::now();
    const auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch());
    return std::to_string(millis.count());
}

static std::filesystem::path test_database_path(
    const std::string& timestamp,
    const std::string& test_name,
    const std::string& db_name) {
    return std::filesystem::path(IP_INVENTORY_TEST_DATA_ROOT) /
           timestamp /
           test_name /
           (timestamp + "." + db_name + ".sqlite3");
}

static void assert_sqlite_ok(const int result, sqlite3* db, const char* operation) {
    if (result != SQLITE_OK && result != SQLITE_DONE && result != SQLITE_ROW) {
        TEST_FAIL_MESSAGE((std::string(operation) + ": " + sqlite3_errmsg(db)).c_str());
    }
}

static void sqlite_database_initializes() {
    const std::string timestamp = test_run_timestamp();
    const std::string testName = "ip_inventory_test_sqlite";
    const std::string dbName = "ip_inventory_test";
    const auto dbPath = test_database_path(timestamp, testName, dbName);
    testRunDirectory = std::filesystem::path(IP_INVENTORY_TEST_DATA_ROOT) / timestamp;

    std::filesystem::create_directories(dbPath.parent_path());

    sqlite3* db = nullptr;
    assert_sqlite_ok(sqlite3_open(dbPath.string().c_str(), &db), db, "open database");

    assert_sqlite_ok(sqlite3_exec(
        db,
        "create table if not exists ip_pool ("
        "ip text primary key,"
        "ip_type text not null,"
        "state text not null default 'available'"
        ")",
        nullptr,
        nullptr,
        nullptr),
        db,
        "create ip_pool");

    const std::string ip = "203.0.113.10";
    const std::string ipType = "IPv4";

    sqlite3_stmt* insert = nullptr;
    assert_sqlite_ok(sqlite3_prepare_v2(
        db,
        "insert or ignore into ip_pool (ip, ip_type) values (?, ?)",
        -1,
        &insert,
        nullptr),
        db,
        "prepare insert");
    assert_sqlite_ok(sqlite3_bind_text(insert, 1, ip.c_str(), -1, SQLITE_TRANSIENT), db, "bind insert ip");
    assert_sqlite_ok(sqlite3_bind_text(insert, 2, ipType.c_str(), -1, SQLITE_TRANSIENT), db, "bind insert ip_type");
    assert_sqlite_ok(sqlite3_step(insert), db, "execute insert");
    sqlite3_finalize(insert);

    sqlite3_stmt* select = nullptr;
    assert_sqlite_ok(sqlite3_prepare_v2(
        db,
        "select ip, ip_type, state from ip_pool where ip = ?",
        -1,
        &select,
        nullptr),
        db,
        "prepare select");
    assert_sqlite_ok(sqlite3_bind_text(select, 1, ip.c_str(), -1, SQLITE_TRANSIENT), db, "bind select ip");

    TEST_ASSERT_EQUAL_INT(SQLITE_ROW, sqlite3_step(select));
    TEST_ASSERT_EQUAL_STRING(ip.c_str(), reinterpret_cast<const char*>(sqlite3_column_text(select, 0)));
    TEST_ASSERT_EQUAL_STRING(ipType.c_str(), reinterpret_cast<const char*>(sqlite3_column_text(select, 1)));
    TEST_ASSERT_EQUAL_STRING("available", reinterpret_cast<const char*>(sqlite3_column_text(select, 2)));
    TEST_ASSERT_EQUAL_INT(SQLITE_DONE, sqlite3_step(select));
    sqlite3_finalize(select);

    TEST_ASSERT_TRUE(std::filesystem::exists(dbPath));
    assert_sqlite_ok(sqlite3_close(db), db, "close database");
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(sqlite_database_initializes);
    return UNITY_END();
}
