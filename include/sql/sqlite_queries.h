#pragma once

namespace ip_inv {

constexpr const char* sqliteEnableForeignKeys = R"sql(
pragma foreign_keys = on;
)sql";

constexpr const char* sqliteCreateIpPoolTable = R"sql(
create table if not exists ip_pool (
    ip_type text not null,
    ip_bytes blob not null,
    display_ip text not null,
    state text not null default 'available',
    primary key (ip_type, ip_bytes)
);
)sql";

constexpr const char* sqliteInsertIpPoolAddress = R"sql(
insert or ignore into ip_pool (ip_type, ip_bytes, display_ip)
values (?, ?, ?);
)sql";

} // namespace ip_inv
