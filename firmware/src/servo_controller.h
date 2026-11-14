#pragma once

#include "config.h"
#include <stdint.h>


class ServoController {
public:
    ServoController();

    // Attach servo to PIN_SERVO. Call once from setup().
    void begin();

    // Set damper position from 0-100% (0=closed, 100=open).
    // Uses calibrated DAMPER_CLOSED_US..DAMPER_OPEN_US pulses.
    void setPosition(float percent);

    // Legacy nominal 0-180 angle command, using SERVO_MIN_US..SERVO_MAX_US.
    // Use setPosition() for the calibrated damper stroke.
    void setAngle(uint8_t angleDeg);

    // Logical angle (0-90 for the damper), not measured shaft position
    uint8_t getCurrentAngle() const;

    // Last commanded pulse; zero means detached/no pulse output.
    uint16_t getCurrentPulseUs() const { return _currentPulseUs; }

    static uint16_t pulseForPosition(float percent, uint16_t closedUs = DAMPER_CLOSED_US,
                                    uint16_t openUs = DAMPER_OPEN_US);

    // Current position as a percentage (0-100)
    float getCurrentPositionPct() const;

    // Detach the servo signal to avoid jitter when not actively moving
    void detach();

private:
    // Write microsecond pulse value to the servo
    void writeMicroseconds(uint16_t us);

    // Map angle to microseconds for precise control
    uint16_t angleToMicroseconds(float angle) const;

    uint8_t _currentAngle;
    uint16_t _currentPulseUs;
    bool    _attached;
};
