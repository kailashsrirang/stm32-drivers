#include "timer.h"
#include "stm32f407.h"

static bool timerGetInfo(const Timer_Registers *timer, uint32_t *clockMask, uint32_t *resetMask, uint32_t *maxCount)
{
    if (timer == TIM2)
    {
        *clockMask = RCC_TIM2EN;
        *resetMask = RCC_TIM2RST;
        *maxCount = UINT32_MAX;
    }
    else if (timer == TIM3)
    {
        *clockMask = RCC_TIM3EN;
        *resetMask = RCC_TIM3RST;
        *maxCount = UINT16_MAX;
    }
    else if (timer == TIM4)
    {
        *clockMask = RCC_TIM4EN;
        *resetMask = RCC_TIM4RST;
        *maxCount = UINT16_MAX;
    }
    else if (timer == TIM5)
    {
        *clockMask = RCC_TIM5EN;
        *resetMask = RCC_TIM5RST;
        *maxCount = UINT32_MAX;
    }
    else
    {
        return false;
    }
    return true;
}

static bool timerIsClockEnabled(const Timer_Registers *timer)
{
    uint32_t clockMask;
    uint32_t resetMask;
    uint32_t maxCount;

    if (!timerGetInfo(timer, &clockMask, &resetMask, &maxCount))
    {
        return false;
    }

    return (RCC->APB1ENR & clockMask) != 0U;
}

bool timerInit(Timer_Registers *timer, uint32_t timerClockHz, uint32_t counterHz, uint32_t autoReload)
{
    uint32_t clockMask;
    uint32_t resetMask;
    uint32_t maxCount;
    if (!timerGetInfo(timer, &clockMask, &resetMask, &maxCount))
    {
        return false;
    }
    // counterHz = (timerClockHz)/ (PSC + 1)
    // =>
    // PSC = (  (timerClockHz)/ (counterHz )  ) - 1
    if (counterHz == 0U || // cant divide by 0
        timerClockHz < counterHz ||
        (timerClockHz % counterHz) != 0U) // needs to be an integer
    {
        return false;
    }

    uint32_t psc = (timerClockHz / counterHz) - 1U;
    // psc is 16 bits
    // arr = 0 blocks counting so reject it
    if (psc > UINT16_MAX ||
        autoReload == 0U ||
        autoReload > maxCount)
    {
        return false;
    }
    // enable clock
    RCC->APB1ENR |= clockMask;

    // reset peripheral
    RCC->APB1RSTR |= resetMask;
    RCC->APB1RSTR &= ~resetMask;

    // Edge-aligned up-counting, initially stopped
    timer->CR1 = 0U;

    timer->PSC = psc;
    timer->ARR = autoReload;

    // reinit counter, this will trigger an interrupt
    timer->EGR = TIM_EGR_UG;

    timer->SR = 0U;
    return true;
}

bool timerStart(Timer_Registers *timer)
{
    uint32_t clockMask;
    uint32_t resetMask;
    uint32_t maxCount;
    if (!timerGetInfo(timer, &clockMask, &resetMask, &maxCount))
    {
        return false;
    }
    timer->CR1 |= TIM_CR1_CEN;
    return true;
}

bool timerStop(Timer_Registers *timer)
{
    uint32_t clockMask;
    uint32_t resetMask;
    uint32_t maxCount;
    if (!timerGetInfo(timer, &clockMask, &resetMask, &maxCount))
    {
        return false;
    }
    timer->CR1 &= ~TIM_CR1_CEN;
    return true;
}

bool timerReadCounter(const Timer_Registers *timer, uint32_t *count)
{
    if (count == NULL || !timerIsClockEnabled(timer))
    {
        return false;
    }

    *count = timer->CNT;
    return true;
}
