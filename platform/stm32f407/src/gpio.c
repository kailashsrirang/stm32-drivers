#include "gpio.h"
#include <stddef.h>

// Checks if the clock for the given GPIO port is enabled. The pointer must be validated before calling this function.
static bool gpioIsClockEnabled(const GPIO_Registers *gpio)
{
    uintptr_t address = (uintptr_t)gpio;
    uintptr_t offset = (address - GPIOA_BASE);

    uint32_t port_idx = offset / GPIO_PORT_SPACING;

    uint32_t mask = 1U << port_idx;
    return (RCC->AHB1ENR & mask) != 0U;
}
// Checks if the given GPIO port is valid
static bool gpioIsValidPort(const GPIO_Registers *gpio)
{
    if (gpio == NULL)
    {
        return false;
    }
    uintptr_t address = (uintptr_t)gpio;
    if ((GPIOA_BASE > address || address > GPIOI_BASE))
    {
        return false;
    }
    uintptr_t offset = (address - GPIOA_BASE);

    if ((offset % GPIO_PORT_SPACING != 0))
    {
        return false;
    }
    return true;
}
bool gpioEnableClock(GPIO_Registers *gpio)
{
    if (!gpioIsValidPort(gpio))
    {
        return false;
    }

    uintptr_t address = (uintptr_t)gpio;
    uintptr_t offset = (address - GPIOA_BASE);

    uint32_t port_idx = offset / GPIO_PORT_SPACING;

    uint32_t mask = 1U << port_idx;
    RCC->AHB1ENR |= mask;
    (void)RCC->AHB1ENR;
    return true;
}

bool gpioSetMode(GPIO_Registers *gpio, uint8_t pin, GpioMode mode)
{
    if (!gpioIsValidPort(gpio) || !gpioIsClockEnabled(gpio))
    {
        return false;
    }
    if (pin >= GPIO_PIN_COUNT || (uint32_t)mode > (uint32_t)GPIO_MODE_ANALOG)
    {
        return false;
    }
    uint32_t shift = ((uint32_t)pin * 2U);
    uint32_t mask = 3U << shift;

    uint32_t value = gpio->MODER;
    value &= ~mask; // clear bits
    value |= (uint32_t)mode << shift;
    gpio->MODER = value;

    return true;
}

bool gpioWritePin(GPIO_Registers *gpio, uint8_t pin, bool value)
{
    if (!gpioIsValidPort(gpio) || !gpioIsClockEnabled(gpio))
    {
        return false;
    }

    if (pin >= GPIO_PIN_COUNT)
    {
        return false;
    }
    uint32_t mask = 1U << (uint32_t)pin;

    if (value)
    {
        gpio->BSRR = mask; // Lower 16 bits set outputs
    }
    else
    {
        gpio->BSRR = mask << 16U; // Upper 16 bits reset outputs
    }

    return true;
}

bool gpioTogglePin(GPIO_Registers *gpio, uint8_t pin)
{

    if (!gpioIsValidPort(gpio) || !gpioIsClockEnabled(gpio))
    {
        return false;
    }

    if (pin >= GPIO_PIN_COUNT)
    {
        return false;
    }
    uint32_t mask = 1U << (uint32_t)pin;

    if ((gpio->ODR & mask) != 0U)
    {
        gpio->BSRR = mask << 16U;
    }
    else
    {
        gpio->BSRR = mask;
    }

    return true;
}

bool gpioReadPin(const GPIO_Registers *gpio, uint8_t pin, bool *value)
{
    if (!gpioIsValidPort(gpio) || !gpioIsClockEnabled(gpio))
    {
        return false;
    }

    if (value == NULL || pin >= GPIO_PIN_COUNT)
    {
        return false;
    }
    uint32_t mask = (1U << (uint32_t)pin);
    *value = (gpio->IDR & mask) != 0U;

    return true;
}

bool gpioConfigureOutput(GPIO_Registers *gpio, uint8_t pin, GpioOutputType type, GpioSpeed speed, GpioPull pull, bool initialValue)
{
    if (!gpioIsValidPort(gpio) || !gpioIsClockEnabled(gpio))
    {
        return false;
    }

    if (
        (uint32_t)type > (uint32_t)GPIO_OUTPUT_OPEN_DRAIN ||
        (uint32_t)speed > (uint32_t)GPIO_SPEED_HIGH ||
        (uint32_t)pull > (uint32_t)GPIO_PULL_DOWN ||
        pin >= GPIO_PIN_COUNT)
    {
        return false;
    }

    gpioWritePin(gpio, pin, initialValue);

    uint32_t shift = ((uint32_t)pin);
    uint32_t mask = (uint32_t)type << shift;
    uint32_t clear_mask = ~(1U << shift);
    uint32_t value = gpio->OTYPER;
    value &= clear_mask; // clear bits
    value |= mask;
    gpio->OTYPER = value;

    shift = ((uint32_t)pin * 2U);
    uint32_t speed_mask = (uint32_t)speed << shift;
    clear_mask = ~(3U << shift);

    value = gpio->OSPEEDR;
    value &= clear_mask; // clear bits
    value |= speed_mask;
    gpio->OSPEEDR = value;

    shift = ((uint32_t)pin * 2U);
    uint32_t pull_mask = (uint32_t)pull << shift;
    clear_mask = ~(3U << shift);

    value = gpio->PUPDR;
    value &= clear_mask; // clear bits
    value |= pull_mask;
    gpio->PUPDR = value;

    return gpioSetMode(gpio, pin, GPIO_MODE_OUTPUT);
}
