#include <stdbool.h>
#include <stdint.h>

#include "gpio.h"
#include "timer.h"
#include "i2c.h"

/* Inspect these in GDB. */
volatile bool setupOk = false;
volatile bool testDone = false;
volatile I2cStatus sensorStatus = I2C_STATUS_NOT_READY;
volatile uint8_t sensorModelId = 0U;

static uint32_t getTimeUs(void)
{
    /* TIM2 counts at 1 MHz: one count per microsecond. */
    return TIM2->CNT;
}

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

    if (!gpioEnableClock(GPIOB))
    {
        halt();
    }

    /* PB8: I2C1 SCL. */
    if (!gpioConfigureAlternate(
            GPIOB, 8U, GPIO_AF4,
            GPIO_OUTPUT_OPEN_DRAIN,
            GPIO_SPEED_LOW, GPIO_PULL_NONE))
    {
        halt();
    }

    /* PB7: I2C1 SDA. */
    if (!gpioConfigureAlternate(
            GPIOB, 7U, GPIO_AF4,
            GPIO_OUTPUT_OPEN_DRAIN,
            GPIO_SPEED_LOW, GPIO_PULL_NONE))
    {
        halt();
    }

    if (!i2cInit(I2C1, 16000000U, 100000U))
    {
        halt();
    }

    setupOk = true;

    /* Allow some startup time before communicating. */
    uint32_t startUs = getTimeUs();

    while ((uint32_t)(getTimeUs() - startUs) < 10000U)
    {
    }

    uint8_t registerAddress = 0xC0U;
    uint8_t modelId = 0U;

    sensorStatus = i2cMasterWriteRead(
        I2C1,
        0x29U,
        &registerAddress,
        1U,
        &modelId,
        1U,
        getTimeUs,
        50000U);

    sensorModelId = modelId;
    testDone = true;

    halt();
}