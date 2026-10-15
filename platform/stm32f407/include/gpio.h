#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32f407.h"

typedef enum
{
    GPIO_MODE_INPUT = 0U,
    GPIO_MODE_OUTPUT = 1U,
    GPIO_MODE_ALTERNATE = 2U,
    GPIO_MODE_ANALOG = 3U
} GpioMode;

typedef enum
{
    GPIO_OUTPUT_PUSH_PULL = 0U,
    GPIO_OUTPUT_OPEN_DRAIN = 1U
} GpioOutputType;

typedef enum
{
    GPIO_SPEED_LOW = 0U,
    GPIO_SPEED_MEDIUM = 1U,
    GPIO_SPEED_FAST = 2U,
    GPIO_SPEED_HIGH = 3U
} GpioSpeed;

typedef enum
{
    GPIO_PULL_NONE = 0U,
    GPIO_PULL_UP = 1U,
    GPIO_PULL_DOWN = 2U
} GpioPull;

bool gpioEnableClock(GPIO_Registers *gpio);
bool gpioSetMode(GPIO_Registers *gpio, uint8_t pin, GpioMode mode);
bool gpioWritePin(GPIO_Registers *gpio, uint8_t pin, bool value);
bool gpioTogglePin(GPIO_Registers *gpio, uint8_t pin);
bool gpioReadPin(const GPIO_Registers *gpio, uint8_t pin, bool *value);
bool gpioConfigureOutput(GPIO_Registers *gpio, uint8_t pin, GpioOutputType type, GpioSpeed speed, GpioPull pull, bool initialValue);

#endif