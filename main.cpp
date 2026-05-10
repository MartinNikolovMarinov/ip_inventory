#include "types.h"

#include <drogon/orm/DbClient.h>

#include <iostream>
#include <string>

using namespace ip_inv;

i32 main() {
    try {
        auto db = drogon::orm::DbClient::newSqlite3Client(
            "filename=ip_inventory_demo.sqlite3", 1);

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

        for (const auto& row : selected) {
            std::cout << "Selected IP: "
                      << row["ip"].as<std::string>() << " "
                      << row["ip_type"].as<std::string>() << " "
                      << row["state"].as<std::string>() << '\n';
        }

        db->execSqlSync("delete from ip_pool where ip = ?", ip);
        std::cout << "Deleted demo IP: " << ip << '\n';

        return 0;
    }
    catch (const std::exception& error) {
        std::cerr << "Database demo failed: " << error.what() << '\n';
        return 1;
    }
}
