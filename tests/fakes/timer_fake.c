#include "timer_fake.h"

#include <string.h>

static TimerFakeState state;

void timerFakeReset(bool result)
{
    memset(&state, 0, sizeof(state));
    state.result = result;
}

TimerFakeState timerFakeGetState(void)
{
    return state;
}

bool timerSetCompare(Timer_Registers *timer, uint8_t channel, uint32_t compareTicks)
{
    state.timer = timer;
    state.channel = channel;
    state.compareTicks = compareTicks;
    state.callCount++;
    return state.result;
}
