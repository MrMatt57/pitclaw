#include <unity.h>
#include "servo_controller.h"
#include "servo_controller.cpp"
#include "servo_pwm.h"
void setUp() {}
void tearDown() {}
void test_boot_and_endpoints_use_calibrated_pulses() {
    ServoController servo;
    TEST_ASSERT_EQUAL_UINT16(0, servo.getCurrentPulseUs());
    servo.begin();
    TEST_ASSERT_EQUAL_UINT16(DAMPER_CLOSED_US, servo.getCurrentPulseUs());
    servo.setPosition(100);
    TEST_ASSERT_EQUAL_UINT16(DAMPER_OPEN_US, servo.getCurrentPulseUs());
    TEST_ASSERT_EQUAL_UINT8(DAMPER_OPEN, servo.getCurrentAngle());
    servo.setPosition(0);
    TEST_ASSERT_EQUAL_UINT16(DAMPER_CLOSED_US, servo.getCurrentPulseUs());
}
void test_interpolation_and_reverse_endpoints() {
    TEST_ASSERT_EQUAL_UINT16(1250, ServoController::pulseForPosition(50, 1000, 1500));
    TEST_ASSERT_EQUAL_UINT16(1800, ServoController::pulseForPosition(0, 1800, 1200));
    TEST_ASSERT_EQUAL_UINT16(1500, ServoController::pulseForPosition(50, 1800, 1200));
    TEST_ASSERT_EQUAL_UINT16(1200, ServoController::pulseForPosition(100, 1800, 1200));
}
void test_limits_invalid_input_and_detach() {
    ServoController servo;
    servo.setPosition(-20);
    TEST_ASSERT_EQUAL_UINT16(DAMPER_CLOSED_US, servo.getCurrentPulseUs());
    servo.setPosition(200);
    TEST_ASSERT_EQUAL_UINT16(DAMPER_OPEN_US, servo.getCurrentPulseUs());
    servo.setPosition(NAN);
    TEST_ASSERT_EQUAL_UINT16(DAMPER_CLOSED_US, servo.getCurrentPulseUs());
    servo.detach();
    TEST_ASSERT_EQUAL_UINT16(0, servo.getCurrentPulseUs());
}
void test_pulse_duty_matches_s3_14bit_timer() {
    TEST_ASSERT_EQUAL(14, SERVO_PWM_RESOLUTION);
    TEST_ASSERT_EQUAL_UINT32(819, servoPulseToDuty(1000));
    TEST_ASSERT_EQUAL_UINT32(1229, servoPulseToDuty(1500));
    TEST_ASSERT_EQUAL_UINT32(1638, servoPulseToDuty(2000));
}
int main() {
    UNITY_BEGIN();
    RUN_TEST(test_boot_and_endpoints_use_calibrated_pulses);
    RUN_TEST(test_interpolation_and_reverse_endpoints);
    RUN_TEST(test_limits_invalid_input_and_detach);
    RUN_TEST(test_pulse_duty_matches_s3_14bit_timer);
    return UNITY_END();
}
