#ifndef SERVO_H
#define SERVO_H

#include <stdbool.h>
#include <stdint.h>

#include "timer.h"

typedef struct
{
    Timer_Registers *timer;
    uint8_t channel;
    uint32_t counterHz;  // number of ticks per second
    uint32_t minPulseUs; // Minimum pulse width in microseconds
    uint32_t maxPulseUs; // Maximum pulse width in microseconds
} Servo;
// pulseUs = dduration output remains high for in microsecs

bool servoSetPulseUs(const Servo *servo, uint32_t pulseUs);
bool servoSetPosition(const Servo *servo, uint16_t position);

#endif