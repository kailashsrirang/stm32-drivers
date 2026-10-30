#ifndef TIMER_H
#define TIMER_H

#include <stdbool.h>
#include <stdint.h>

#include "stm32f407.h"

bool timerInit(Timer_Registers *timer, uint32_t timerClockHz, uint32_t counterHz, uint32_t autoReload);

bool timerStart(Timer_Registers *timer);
bool timerStop(Timer_Registers *timer);

bool timerReadCounter(const Timer_Registers *timer, uint32_t *count);

#endif