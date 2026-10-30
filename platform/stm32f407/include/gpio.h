#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32f407.h"

typedef enum
{
    GPIO_AF0 = 0U,
    GPIO_AF1 = 1U,
    GPIO_AF2 = 2U,
    GPIO_AF3 = 3U,
    GPIO_AF4 = 4U,
    GPIO_AF5 = 5U,
    GPIO_AF6 = 6U,
    GPIO_AF7 = 7U,
    GPIO_AF8 = 8U,
    GPIO_AF9 = 9U,
    GPIO_AF10 = 10U,
    GPIO_AF11 = 11U,
    GPIO_AF12 = 12U,
    GPIO_AF13 = 13U,
    GPIO_AF14 = 14U,
    GPIO_AF15 = 15U
} GPIO_AlternateFunction;

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
bool gpioConfigureAlternate(GPIO_Registers *gpio, uint8_t pin, GPIO_AlternateFunction alternateFunction, GpioOutputType type, GpioSpeed speed, GpioPull pull);

#endif