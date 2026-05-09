#include "types.h"

#include <type_traits>
#include <unity.h>

void setUp() {}

void tearDown() {}

static void integer_aliases_have_expected_sizes() {
    TEST_ASSERT_EQUAL_UINT64(1, sizeof(ip_inv::i8));
    TEST_ASSERT_EQUAL_UINT64(2, sizeof(ip_inv::i16));
    TEST_ASSERT_EQUAL_UINT64(4, sizeof(ip_inv::i32));
    TEST_ASSERT_EQUAL_UINT64(8, sizeof(ip_inv::i64));
}

static void rawbytes_matches_wrapped_type_size() {
    TEST_ASSERT_EQUAL_UINT64(sizeof(ip_inv::u64), sizeof(ip_inv::rawbytes<ip_inv::u64>));
    TEST_ASSERT_TRUE((std::is_trivially_copyable_v<ip_inv::rawbytes<ip_inv::u64>>));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(integer_aliases_have_expected_sizes);
    RUN_TEST(rawbytes_matches_wrapped_type_size);
    return UNITY_END();
}
