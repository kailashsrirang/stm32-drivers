#ifndef STM32F407_H
#define STM32F407_H

#include <stdint.h>
#include <stddef.h>

// RCC base address
#define RCC_BASE 0x40023800U

#define RCC ((Rcc_Registers *)((RCC_BASE)))

#define RCC_GPIOAEN (1U << 0)
#define RCC_GPIOBEN (1U << 1)
#define RCC_GPIOCEN (1U << 2)
#define RCC_GPIODEN (1U << 3)
#define RCC_GPIOEEN (1U << 4)
#define RCC_GPIOFEN (1U << 5)
#define RCC_GPIOGEN (1U << 6)
#define RCC_GPIOHEN (1U << 7)
#define RCC_GPIOIEN (1U << 8)

// GPIO base addresses
#define GPIOA_BASE 0x40020000U
#define GPIOB_BASE 0x40020400U
#define GPIOC_BASE 0x40020800U
#define GPIOD_BASE 0x40020C00U
#define GPIOE_BASE 0x40021000U
#define GPIOF_BASE 0x40021400U
#define GPIOG_BASE 0x40021800U
#define GPIOH_BASE 0x40021C00U
#define GPIOI_BASE 0x40022000U

#define GPIO_MODER_OFFSET 0x00U
#define GPIO_OTYPER_OFFSET 0x04U
#define GPIO_OSPEEDR_OFFSET 0x08U
#define GPIO_PUPDR_OFFSET 0x0CU
#define GPIO_IDR_OFFSET 0x10U
#define GPIO_ODR_OFFSET 0x14U
#define GPIO_BSRR_OFFSET 0x18U
#define GPIO_LCKR_OFFSET 0x1CU
#define GPIO_AFRL_OFFSET 0x20U
#define GPIO_AFRH_OFFSET 0x24U

#define GPIO_PIN_COUNT 16U
#define GPIO_PORT_SPACING 0x0400UL

typedef struct
{
    volatile uint32_t MODER;   // GPIO port mode register
    volatile uint32_t OTYPER;  // GPIO port output type register
    volatile uint32_t OSPEEDR; // GPIO port output speed register
    volatile uint32_t PUPDR;   // GPIO port pull-up/pull-down register
    volatile uint32_t IDR;     // GPIO port input data register
    volatile uint32_t ODR;     // GPIO port output data register
    volatile uint32_t BSRR;    // GPIO port bit set/reset register
    volatile uint32_t LCKR;    // GPIO port configuration lock register
    volatile uint32_t AFRL;    // GPIO alternate function low register
    volatile uint32_t AFRH;    // GPIO alternate function high register
} GPIO_Registers;

_Static_assert(offsetof(GPIO_Registers, MODER) == 0x00U, "GPIO MODER offset is incorrect");
_Static_assert(offsetof(GPIO_Registers, BSRR) == 0x18U, "GPIO BSRR offset is incorrect");
_Static_assert(offsetof(GPIO_Registers, AFRH) == 0x24U, "GPIO AFRH offset is incorrect");
_Static_assert(sizeof(GPIO_Registers) == 0x28U, "GPIO register structure size is incorrect");

typedef struct
{
    volatile uint32_t CR;       /* Offset 0x00 */
    volatile uint32_t PLLCFGR;  /* Offset 0x04 */
    volatile uint32_t CFGR;     /* Offset 0x08 */
    volatile uint32_t CIR;      /* Offset 0x0C */
    volatile uint32_t AHB1RSTR; /* Offset 0x10 */
    volatile uint32_t AHB2RSTR; /* Offset 0x14 */
    volatile uint32_t AHB3RSTR; /* Offset 0x18 */
    uint32_t RESERVED0;         /* Offset 0x1C */
    volatile uint32_t APB1RSTR; /* Offset 0x20 */
    volatile uint32_t APB2RSTR; /* Offset 0x24 */
    uint32_t RESERVED1[2];      /* Offsets 0x28–0x2C */
    volatile uint32_t AHB1ENR;  /* Offset 0x30 */
} Rcc_Registers;

_Static_assert(offsetof(Rcc_Registers, AHB1ENR) == 0x30U, "RCC AHB1ENR offset is incorrect");

#define GPIOA ((GPIO_Registers *)GPIOA_BASE)
#define GPIOB ((GPIO_Registers *)GPIOB_BASE)
#define GPIOC ((GPIO_Registers *)GPIOC_BASE)
#define GPIOD ((GPIO_Registers *)GPIOD_BASE)
#define GPIOE ((GPIO_Registers *)GPIOE_BASE)
#define GPIOF ((GPIO_Registers *)GPIOF_BASE)
#define GPIOG ((GPIO_Registers *)GPIOG_BASE)
#define GPIOH ((GPIO_Registers *)GPIOH_BASE)
#define GPIOI ((GPIO_Registers *)GPIOI_BASE)

#endif