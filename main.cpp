#include "types.h"

#include <httplib.h>
#include <sqlite3.h>

#include <iostream>
#include <stdexcept>
#include <string>

using namespace ip_inv;

static void throw_on_sqlite_error(
    const int result,
    sqlite3* db,
    const std::string& operation) {
    if (result != SQLITE_OK && result != SQLITE_DONE && result != SQLITE_ROW) {
        throw std::runtime_error(operation + ": " + sqlite3_errmsg(db));
    }
}

static void verify_http_server_integration() {
    httplib::Server server;
    server.Get("/health", [](const httplib::Request&, httplib::Response& response) {
        response.set_content("ok", "text/plain");
    });

    const int port = server.bind_to_any_port("127.0.0.1");
    if (port <= 0) {
        throw std::runtime_error("bind httplib server");
    }
    server.stop();
    std::cout << "Valid http server" << '\n';
}

i32 main() {
    sqlite3* db = nullptr;

    try {
        verify_http_server_integration();

        throw_on_sqlite_error(
            sqlite3_open("ip_inventory_demo.sqlite3", &db),
            db,
            "open database");

        throw_on_sqlite_error(sqlite3_exec(
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
        throw_on_sqlite_error(sqlite3_prepare_v2(
            db,
            "insert or ignore into ip_pool (ip, ip_type) values (?, ?)",
            -1,
            &insert,
            nullptr),
            db,
            "prepare insert");
        throw_on_sqlite_error(sqlite3_bind_text(insert, 1, ip.c_str(), -1, SQLITE_TRANSIENT), db, "bind insert ip");
        throw_on_sqlite_error(sqlite3_bind_text(insert, 2, ipType.c_str(), -1, SQLITE_TRANSIENT), db, "bind insert ip_type");
        throw_on_sqlite_error(sqlite3_step(insert), db, "execute insert");
        sqlite3_finalize(insert);

        sqlite3_stmt* select = nullptr;
        throw_on_sqlite_error(sqlite3_prepare_v2(
            db,
            "select ip, ip_type, state from ip_pool where ip = ?",
            -1,
            &select,
            nullptr),
            db,
            "prepare select");
        throw_on_sqlite_error(sqlite3_bind_text(select, 1, ip.c_str(), -1, SQLITE_TRANSIENT), db, "bind select ip");

        while (sqlite3_step(select) == SQLITE_ROW) {
            std::cout << "Selected IP: "
                      << sqlite3_column_text(select, 0) << " "
                      << sqlite3_column_text(select, 1) << " "
                      << sqlite3_column_text(select, 2) << '\n';
        }
        sqlite3_finalize(select);

        sqlite3_stmt* remove = nullptr;
        throw_on_sqlite_error(sqlite3_prepare_v2(
            db,
            "delete from ip_pool where ip = ?",
            -1,
            &remove,
            nullptr),
            db,
            "prepare delete");
        throw_on_sqlite_error(sqlite3_bind_text(remove, 1, ip.c_str(), -1, SQLITE_TRANSIENT), db, "bind delete ip");
        throw_on_sqlite_error(sqlite3_step(remove), db, "execute delete");
        sqlite3_finalize(remove);

        std::cout << "Deleted demo IP: " << ip << '\n';

        sqlite3_close(db);
        return 0;
    }
    catch (const std::exception& error) {
        if (db != nullptr) {
            sqlite3_close(db);
        }
        std::cerr << "Database demo failed: " << error.what() << '\n';
        return 1;
    }
}
