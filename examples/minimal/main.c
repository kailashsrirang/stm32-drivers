#include <stdbool.h>
#include <stdint.h>

#include "gpio.h"
#include "timer.h"

static void halt(void)
{
    while (1)
    {
    }
}

int main(void)
{
    if (!timerInit(TIM2, 16000000U, 1000000U, UINT32_MAX) ||
        !timerStart(TIM2))
    {
        halt();
    }

    if (!gpioEnableClock(GPIOD) ||
        !gpioConfigureOutput(GPIOD, 12U, GPIO_OUTPUT_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE, false))
    {
        halt();
    }

    uint32_t previous;

    if (!timerReadCounter(TIM2, &previous))
    {
        halt();
    }

    while (1)
    {
        uint32_t now;

        if (!timerReadCounter(TIM2, &now))
        {
            halt();
        }

        if ((uint32_t)(now - previous) >= 500000U)
        {
            previous = now;
            gpioTogglePin(GPIOD, 12U);
        }
    }
}