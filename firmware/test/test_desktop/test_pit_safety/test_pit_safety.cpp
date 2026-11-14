#include <unity.h>
#include "pit_safety.h"
void setUp() {}
void tearDown() {}
void test_startup_holds_outputs_off() {
    PitSafety safety;
    TEST_ASSERT_FALSE(safety.ready());
    TEST_ASSERT_FALSE(safety.update(false, 0));
    TEST_ASSERT_FALSE(safety.update(true, 1000));
    TEST_ASSERT_FALSE(safety.update(true, 3999));
    TEST_ASSERT_TRUE(safety.update(true, 4000));
}
void test_probe_loss_immediately_inhibits_and_reconnect_must_settle() {
    PitSafety safety;
    safety.update(true, 100);
    TEST_ASSERT_TRUE(safety.update(true, 3100));
    TEST_ASSERT_FALSE(safety.update(false, 3101));
    TEST_ASSERT_FALSE(safety.update(true, 3200));
    TEST_ASSERT_FALSE(safety.update(false, 4000));
    TEST_ASSERT_FALSE(safety.update(true, 4100));
    TEST_ASSERT_FALSE(safety.update(true, 7099));
    TEST_ASSERT_TRUE(safety.update(true, 7100));
}
void test_recovery_timer_handles_millis_wrap() {
    PitSafety safety;
    TEST_ASSERT_FALSE(safety.update(true, UINT32_MAX - 1000));
    TEST_ASSERT_FALSE(safety.update(true, 1998));
    TEST_ASSERT_TRUE(safety.update(true, 1999));
}
int main() {
    UNITY_BEGIN();
    RUN_TEST(test_startup_holds_outputs_off);
    RUN_TEST(test_probe_loss_immediately_inhibits_and_reconnect_must_settle);
    RUN_TEST(test_recovery_timer_handles_millis_wrap);
    return UNITY_END();
}
