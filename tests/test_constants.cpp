#include "types.h"

#include <unity.h>

void setUp() {}

void tearDown() {}

static void decimal_storage_constants_scale_by_1000() {
    TEST_ASSERT_EQUAL_UINT64(1000, ip_inv::CORE_KILOBYTE);
    TEST_ASSERT_EQUAL_UINT64(1000 * ip_inv::CORE_KILOBYTE, ip_inv::CORE_MEGABYTE);
    TEST_ASSERT_EQUAL_UINT64(1000 * ip_inv::CORE_MEGABYTE, ip_inv::CORE_GIGABYTE);
}

static void duration_constants_are_in_nanoseconds() {
    TEST_ASSERT_EQUAL_UINT64(1000, ip_inv::CORE_MICROSECOND);
    TEST_ASSERT_EQUAL_UINT64(1000 * ip_inv::CORE_MICROSECOND, ip_inv::CORE_MILLISECOND);
    TEST_ASSERT_EQUAL_UINT64(1000 * ip_inv::CORE_MILLISECOND, ip_inv::CORE_SECOND);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(decimal_storage_constants_scale_by_1000);
    RUN_TEST(duration_constants_are_in_nanoseconds);
    return UNITY_END();
}
