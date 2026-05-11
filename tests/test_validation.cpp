#include <unity.h>

#include "validation.h"

using namespace ip_inv;

void setUp() {}

void tearDown() {}

static void validates_ports() {
    struct TestCase {
        i32 value;
        bool expected;
    };

    const TestCase tests[] = {
        {1, true},
        {80, true},
        {443, true},
        {65534, true},
        {0, false},
        {-1, false},
        {65535, false},
        {70000, false},
    };

    for (const TestCase& test : tests) {
        TEST_ASSERT_EQUAL(test.expected, isValidPort(test.value));
    }
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(validates_ports);
    return UNITY_END();
}
