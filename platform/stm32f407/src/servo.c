#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "servo.h"
#include "timer.h"

bool servoSetPulseUs(const Servo *servo, uint32_t pulseUs)
{
    if (servo == NULL || pulseUs == 0)
    {
        return false;
    }

    uint32_t maxPulse = servo->maxPulseUs;
    uint32_t minPulse = servo->minPulseUs;

    if (minPulse == 0U || minPulse > maxPulse)
    {
        return false;
    }

    if (pulseUs > maxPulse || pulseUs < minPulse)
    {
        return false;
    }
    if (servo->counterHz == 0U)
    {
        return false;
    }

    // ticks = (pulseUs/ 1e^6) * counterHz
    uint64_t compareTicks = ((uint64_t)pulseUs * servo->counterHz) / 1000000U;

    if (compareTicks == 0U || compareTicks > UINT32_MAX)
    {
        return false; // underflow or larger that uint32;
    }

    return timerSetCompare(servo->timer, servo->channel, (uint32_t)compareTicks);
}

bool servoSetPosition(const Servo *servo, uint16_t position)
{

    if (servo == NULL)
    {
        return false;
    }

    if (position > 1000)
    {
        return false;
    }
    if (servo->maxPulseUs < servo->minPulseUs ||
        servo->maxPulseUs == 0 ||
        servo->minPulseUs == 0)
    {
        return false;
    }
    // divide servo position into points of 0-1000
    uint64_t offset = ((uint64_t)(servo->maxPulseUs - servo->minPulseUs) * position) / 1000U;

    uint32_t pulseUs = servo->minPulseUs + (uint32_t)offset;
    servoSetPulseUs(servo, offset);

    return true;
}