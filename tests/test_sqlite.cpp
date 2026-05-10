#include <drogon/orm/DbClient.h>
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

static void sqlite_database_initializes() {
    const std::string timestamp = test_run_timestamp();
    const std::string testName = "ip_inventory_test_sqlite";
    const std::string dbName = "ip_inventory_test";
    const auto dbPath = test_database_path(timestamp, testName, dbName);
    testRunDirectory = std::filesystem::path(IP_INVENTORY_TEST_DATA_ROOT) / timestamp;

    std::filesystem::create_directories(dbPath.parent_path());

    auto db = drogon::orm::DbClient::newSqlite3Client(
        "filename=" + dbPath.string(), 1);

    db->execSqlSync(
        "create table if not exists ip_pool ("
        "ip text primary key,"
        "ip_type text not null,"
        "state text not null default 'available'"
        ")");

    const std::string ip = "203.0.113.10";
    const std::string ipType = "IPv4";

    db->execSqlSync(
        "insert or ignore into ip_pool (ip, ip_type) values (?, ?)",
        ip,
        ipType);

    const auto selected = db->execSqlSync(
        "select ip, ip_type, state from ip_pool where ip = ?",
        ip);

    TEST_ASSERT_EQUAL_UINT64(1, selected.size());
    TEST_ASSERT_EQUAL_STRING(ip.c_str(), selected[0]["ip"].as<std::string>().c_str());
    TEST_ASSERT_EQUAL_STRING(ipType.c_str(), selected[0]["ip_type"].as<std::string>().c_str());
    TEST_ASSERT_EQUAL_STRING("available", selected[0]["state"].as<std::string>().c_str());
    TEST_ASSERT_TRUE(std::filesystem::exists(dbPath));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(sqlite_database_initializes);
    return UNITY_END();
}
