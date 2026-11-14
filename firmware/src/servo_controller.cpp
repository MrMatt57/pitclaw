#include "servo_controller.h"
#include "servo_pwm.h"
#include <math.h>

static_assert(DAMPER_CLOSED_US >= SERVO_MIN_US && DAMPER_CLOSED_US <= SERVO_MAX_US,
              "Closed pulse must stay within the validated servo limits");
static_assert(DAMPER_OPEN_US >= SERVO_MIN_US && DAMPER_OPEN_US <= SERVO_MAX_US,
              "Open pulse must stay within the validated servo limits");

#ifndef NATIVE_BUILD
#include <Arduino.h>
#include <soc/soc_caps.h>
static_assert(SERVO_PWM_RESOLUTION <= SOC_LEDC_TIMER_BIT_WIDE_NUM,
              "Servo PWM resolution exceeds this chip's LEDC capability");
#endif

ServoController::ServoController()
    : _currentAngle(DAMPER_CLOSED)
    , _currentPulseUs(0)
    , _attached(false)
{
}

void ServoController::begin() {
#ifndef NATIVE_BUILD
    if (ledcSetup(SERVO_PWM_CHANNEL, 50, SERVO_PWM_RESOLUTION) == 0) {
        _currentPulseUs = 0;
        Serial.println("[SERVO] PWM setup failed; servo output unavailable.");
        return;
    }
    ledcAttachPin(PIN_SERVO, SERVO_PWM_CHANNEL);
    _attached = true;
    Serial.printf("[SERVO] Attached to pin %d, closed=%dus, open=%dus\n",
                  PIN_SERVO, DAMPER_CLOSED_US, DAMPER_OPEN_US);
#endif
    // Boot and fault shutdown use the same calibrated closed pulse.
    setPosition(0.0f);
}

void ServoController::setPosition(float percent) {
    // Clamp to 0-100%
    if (!isfinite(percent) || percent < 0.0f) percent = 0.0f;
    if (percent > 100.0f) percent = 100.0f;

    // Map 0-100% to DAMPER_CLOSED..DAMPER_OPEN angle range
    float angle = (float)DAMPER_CLOSED +
                  (percent / 100.0f) * (float)(DAMPER_OPEN - DAMPER_CLOSED);

    // Clamp angle to valid range
    if (angle < (float)DAMPER_CLOSED) angle = (float)DAMPER_CLOSED;
    if (angle > (float)DAMPER_OPEN) angle = (float)DAMPER_OPEN;

    _currentAngle = (uint8_t)(angle + 0.5f);

    writeMicroseconds(pulseForPosition(percent));
}

uint16_t ServoController::pulseForPosition(float percent, uint16_t closedUs, uint16_t openUs) {
    if (!isfinite(percent) || percent < 0.0f) percent = 0.0f;
    if (percent > 100.0f) percent = 100.0f;
    return (uint16_t)((float)closedUs + percent * 0.01f *
                      ((float)openUs - (float)closedUs) + 0.5f);
}

void ServoController::setAngle(uint8_t angleDeg) {
    if (angleDeg > 180) angleDeg = 180;
    _currentAngle = angleDeg;

    uint16_t us = angleToMicroseconds((float)angleDeg);
    writeMicroseconds(us);
}

uint8_t ServoController::getCurrentAngle() const {
    return _currentAngle;
}

float ServoController::getCurrentPositionPct() const {
    if (DAMPER_OPEN == DAMPER_CLOSED) return 0.0f;
    float pct = (float)(_currentAngle - DAMPER_CLOSED) /
                (float)(DAMPER_OPEN - DAMPER_CLOSED) * 100.0f;
    if (pct < 0.0f) pct = 0.0f;
    if (pct > 100.0f) pct = 100.0f;
    return pct;
}

void ServoController::detach() {
#ifndef NATIVE_BUILD
    if (_attached) {
        ledcWrite(SERVO_PWM_CHANNEL, 0);
        ledcDetachPin(PIN_SERVO);
        pinMode(PIN_SERVO, OUTPUT);
        digitalWrite(PIN_SERVO, LOW);
        _attached = false;
        Serial.println("[SERVO] Detached.");
    }
#endif
    _currentPulseUs = 0;
}

void ServoController::writeMicroseconds(uint16_t us) {
    _currentPulseUs = us;
#ifndef NATIVE_BUILD
    if (!_attached) {
        if (ledcSetup(SERVO_PWM_CHANNEL, 50, SERVO_PWM_RESOLUTION) == 0) {
            _currentPulseUs = 0;
            return;
        }
        ledcAttachPin(PIN_SERVO, SERVO_PWM_CHANNEL);
        _attached = true;
    }
    ledcWrite(SERVO_PWM_CHANNEL, servoPulseToDuty(us));
#endif
}

uint16_t ServoController::angleToMicroseconds(float angle) const {
    // Linear interpolation from 0-180 degrees to SERVO_MIN_US-SERVO_MAX_US
    // This is the legacy angle API; the damper uses calibrated pulse endpoints.
    if (angle < 0.0f) angle = 0.0f;
    if (angle > 180.0f) angle = 180.0f;

    float us = (float)SERVO_MIN_US +
               (angle / 180.0f) * (float)(SERVO_MAX_US - SERVO_MIN_US);
    return (uint16_t)(us + 0.5f);
}
