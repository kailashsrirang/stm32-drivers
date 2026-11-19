#ifndef TIMER_FAKE_H
#define TIMER_FAKE_H

#include <stdbool.h>
#include <stdint.h>

#include "timer.h"

typedef struct
{
    Timer_Registers *timer;
    uint8_t channel;
    uint32_t compareTicks;
    uint32_t callCount;
    bool result;
} TimerFakeState;

void timerFakeReset(bool result);
TimerFakeState timerFakeGetState(void);

#endif
