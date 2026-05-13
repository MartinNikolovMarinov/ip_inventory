DROP TABLE IF EXISTS ip_pool;
DROP TABLE IF EXISTS reserved_ips;
DROP TABLE IF EXISTS services;

CREATE TABLE services (
    id INTEGER PRIMARY KEY,
    service_id TEXT NOT NULL UNIQUE
);

CREATE TABLE reserved_ips (
    id INTEGER PRIMARY KEY,
    service_id INTEGER NOT NULL, -- TODO: add an index for this to speedup assignment
    expiration_time BIGINT NOT NULL,

    FOREIGN KEY (service_id)
        REFERENCES services(id)
        ON DELETE CASCADE
);

CREATE TABLE ip_pool (
    ip_type INTEGER NOT NULL,
    ip_bytes BLOB NOT NULL,
    display_ip TEXT NOT NULL,

    assigned_id INTEGER,
    reserved_id INTEGER,

    PRIMARY KEY (ip_type, ip_bytes),

    -- Only one of (assigned, reserved) can be non-null
    CHECK (assigned_id IS NULL OR reserved_id IS NULL),

    -- Rules for ipv4 and ipv6
    CHECK (ip_type IN (4, 6)),
    CHECK (
        (ip_type = 4 AND length(ip_bytes) = 4) OR
        (ip_type = 6 AND length(ip_bytes) = 16)
    ),

    FOREIGN KEY (assigned_id)
        REFERENCES services(id)
        ON DELETE SET NULL,

    FOREIGN KEY (reserved_id)
        REFERENCES reserved_ips(id)
        ON DELETE SET NULL
);

CREATE INDEX idx_ip_pool_assigned_id ON ip_pool(assigned_id);

CREATE INDEX idx_ip_pool_reserved_id ON ip_pool(reserved_id);

CREATE INDEX idx_reserved_ips_expiration_time ON reserved_ips(expiration_time); -- speedup the gc
