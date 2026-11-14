#pragma once
#include "config.h"
#include <stdint.h>

// 50Hz has a 20000us period. Shared by production and the bench console.
inline uint32_t servoPulseToDuty(uint16_t pulseUs) {
    return ((uint32_t)pulseUs * (1UL << SERVO_PWM_RESOLUTION) + 10000UL) / 20000UL;
}
