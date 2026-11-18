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

static volatile uint32_t *timerGetCompareRegister(Timer_Registers *timer, uint8_t channel)
{
    uint32_t clockMask;
    uint32_t resetMask;
    uint32_t maxCount;
    if (channel < 1U || channel > 4U || !timerGetInfo(timer, &clockMask, &resetMask, &maxCount))
    {
        return NULL; // Safety guard for invalid channels
    }
    return &timer->CCR1 + (channel - 1U);
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

bool timerConfigurePwm(Timer_Registers *timer, uint8_t channel, uint32_t compareTicks)
{
    if (!timerIsClockEnabled(timer))
    {
        return false;
    }

    volatile uint32_t *ccr_reg = timerGetCompareRegister(timer, channel);

    if (ccr_reg == NULL)
    {
        return false;
    }

    if ((timer->CR1 & TIM_CR1_CEN) != 0)
    {
        return false; // timer is already counting
    }

    if (compareTicks > timer->ARR)
    {
        return false;
    }
    uint32_t enableShift = ((uint32_t)channel - 1U) * 4U;

    // disable this channel and select active-high polarity
    timer->CCER &= ~(0xBU << enableShift);

    volatile uint32_t *mode_register;
    if (channel == 1 || channel == 2)
    {
        mode_register = &timer->CCMR1;
    }
    else
    {
        mode_register = &timer->CCMR2;
    }

    uint32_t modeShift = ((uint32_t)(channel - 1U) % 2U) * 8U;
    uint32_t value =
        (0U << 7U) | // external clearing disabled
        (6U << 4U) | // PWM mode 1. val high when cnt < ccr low after
        (1U << 3U) | // compare preload enabled
        (0U << 2U) | // fast mode disabled
        (0U << 0U);  // output mode

    *mode_register = (*mode_register & ~(0xFFU << modeShift)) | (value << modeShift);

    // write pulse width to preload reg
    *ccr_reg = compareTicks;

    //  generate an update event and load buffered values
    timer->EGR = TIM_EGR_UG;

    // clear update interrupt flag
    timer->SR = 0U;

    // enable output
    timer->CCER |= (1U << enableShift);

    return true;
}

bool timerSetCompare(Timer_Registers *timer, uint8_t channel, uint32_t compareTicks)
{
    if (!timerIsClockEnabled(timer))
    {
        return false;
    }

    volatile uint32_t *ccr_reg = timerGetCompareRegister(timer, channel);
    if (ccr_reg == NULL)
    {
        return false;
    }

    if (compareTicks > timer->ARR)
    {
        return false;
    }

    *ccr_reg = compareTicks;
    return true;
}