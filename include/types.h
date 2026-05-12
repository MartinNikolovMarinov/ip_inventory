#pragma once

#include <stdint.h>
#include <stddef.h>

namespace ip_inv {

using i8    = int8_t;
using i16   = int16_t;
using i32   = int32_t;
using i64   = int64_t;
using u8    = uint8_t;
using u16   = uint16_t;
using u32   = uint32_t;
using u64   = uint64_t;
using f32   = float;
using f64   = double;
using uchar = unsigned char;
using schar = signed char;
using usize = u64;
using isize = i64;
using rune  = u32; // Represents a Unicode code point.

template <typename T>
struct rawbytes { alignas(T) u8 bytes[sizeof(T)]; };

constexpr i32 BYTE_SIZE = 8;

// Storage Sizes
constexpr u64 CORE_BYTE     = u64(1);
constexpr u64 CORE_KILOBYTE = u64(1000 * CORE_BYTE);
constexpr u64 CORE_MEGABYTE = u64(1000 * CORE_KILOBYTE);
constexpr u64 CORE_GIGABYTE = u64(1000 * CORE_MEGABYTE);
constexpr u64 CORE_TERABYTE = u64(1000 * CORE_GIGABYTE);
constexpr u64 CORE_KiB      = u64(1024);
constexpr u64 CORE_MiB      = u64(1024 * CORE_KiB);
constexpr u64 CORE_GiB      = u64(1024 * CORE_MiB);
constexpr u64 CORE_TiB      = u64(1024 * CORE_GiB);

// Duration constants in ns
constexpr u64 CORE_NANOSECOND  = u64(1);                       //                 1ns
constexpr u64 CORE_MICROSECOND = u64(1000 * CORE_NANOSECOND);  //             1_000ns
constexpr u64 CORE_MILLISECOND = u64(1000 * CORE_MICROSECOND); //         1_000_000ns
constexpr u64 CORE_SECOND      = u64(1000 * CORE_MILLISECOND); //     1_000_000_000ns
constexpr u64 CORE_MINUTE      = u64(60 * CORE_SECOND);        //    60_000_000_000ns
constexpr u64 CORE_HOUR        = u64(60 * CORE_MINUTE);        // 3_600_000_000_000ns

enum struct HttpMethod : u8 {
    GET,
    POST,
    PATCH,
    PUT,
    DELETE
};

enum struct HttpStatusCode : u32 {
    Ok = 200,
    BadRequest = 400,
    NotFound = 404,
    InternalServerError = 500
};

} // ip_inv
