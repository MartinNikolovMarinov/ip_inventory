create table if not exists ip_pool (
    ip text primary key,
    ip_type text not null check (ip_type in ('IPv4', 'IPv6')),
    state text not null default 'Free' check (state in ('Free', 'Reserved', 'Assigned')),
    service_id text null,
    reserved_until timestamptz null
);

create index if not exists idx_ip_pool_service_id
    on ip_pool(service_id);

create index if not exists idx_ip_pool_state_type
    on ip_pool(state, ip_type);
