#ifndef STM32F407_H
#define STM32F407_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "stm32f407.h"

// RCC base address
#define RCC_BASE 0x40023800U

#define RCC ((Rcc_Registers *)((RCC_BASE)))

#define I2C1 ((I2C_Registers *)0x40005400UL)
#define I2C2 ((I2C_Registers *)0x40005800UL)
#define I2C3 ((I2C_Registers *)0x40005C00UL)

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

#define TIM2_BASE 0x40000000U
#define TIM3_BASE 0x40000400U
#define TIM4_BASE 0x40000800U
#define TIM5_BASE 0x40000C00U

#define RCC_TIM2EN (1U << 0)
#define RCC_TIM3EN (1U << 1)
#define RCC_TIM4EN (1U << 2)
#define RCC_TIM5EN (1U << 3)

#define RCC_TIM2RST (1U << 0)
#define RCC_TIM3RST (1U << 1)
#define RCC_TIM4RST (1U << 2)
#define RCC_TIM5RST (1U << 3)

#define TIM_CR1_CEN (1U << 0)
#define TIM_EGR_UG (1U << 0)

#define RCC_I2C1EN (1U << 21U)
#define RCC_I2C1RST (1U << 21U)
#define RCC_I2C2EN (1U << 22U)
#define RCC_I2C2RST (1U << 22U)
#define RCC_I2C3EN (1U << 23U)
#define RCC_I2C3RST (1U << 23U)
#define I2C_CR1_PE (1U << 0U)

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

typedef struct
{
    volatile uint32_t CR1;   // 0x00: Control register 1
    volatile uint32_t CR2;   // 0x04: Control register 2
    volatile uint32_t SMCR;  // 0x08: Slave mode control
    volatile uint32_t DIER;  // 0x0C: DMA/interrupt enable
    volatile uint32_t SR;    // 0x10: Status
    volatile uint32_t EGR;   // 0x14: Event generation
    volatile uint32_t CCMR1; // 0x18: Capture/compare mode 1
    volatile uint32_t CCMR2; // 0x1C: Capture/compare mode 2
    volatile uint32_t CCER;  // 0x20: Capture/compare enable
    volatile uint32_t CNT;   // 0x24: Counter
    volatile uint32_t PSC;   // 0x28: Prescaler
    volatile uint32_t ARR;   // 0x2C: Auto-reload
    uint32_t RESERVED0;      // 0x30: Reserved
    volatile uint32_t CCR1;  // 0x34: Capture/compare 1
    volatile uint32_t CCR2;  // 0x38: Capture/compare 2
    volatile uint32_t CCR3;  // 0x3C: Capture/compare 3
    volatile uint32_t CCR4;  // 0x40: Capture/compare 4
} Timer_Registers;

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
    volatile uint32_t AHB2ENR;  /* Offset 0x34 */
    volatile uint32_t AHB3ENR;  /* Offset 0x38 */
    uint32_t RESERVED2;         /* Offset 0x3C */
    volatile uint32_t APB1ENR;  /* Offset 0x40 */
    volatile uint32_t APB2ENR;  /* Offset 0x44 */
} Rcc_Registers;

_Static_assert(offsetof(Rcc_Registers, AHB1ENR) == 0x30U, "RCC AHB1ENR offset is incorrect");

_Static_assert(offsetof(Rcc_Registers, APB1ENR) == 0x40U, "RCC APB1ENR offset is incorrect");

typedef struct
{
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t OAR1;
    volatile uint32_t OAR2;
    volatile uint32_t DR;
    volatile uint32_t SR1;
    volatile uint32_t SR2;
    volatile uint32_t CCR;
    volatile uint32_t TRISE;
} I2C_Registers;

#define GPIOA ((GPIO_Registers *)GPIOA_BASE)
#define GPIOB ((GPIO_Registers *)GPIOB_BASE)
#define GPIOC ((GPIO_Registers *)GPIOC_BASE)
#define GPIOD ((GPIO_Registers *)GPIOD_BASE)
#define GPIOE ((GPIO_Registers *)GPIOE_BASE)
#define GPIOF ((GPIO_Registers *)GPIOF_BASE)
#define GPIOG ((GPIO_Registers *)GPIOG_BASE)
#define GPIOH ((GPIO_Registers *)GPIOH_BASE)
#define GPIOI ((GPIO_Registers *)GPIOI_BASE)

#define TIM2 ((Timer_Registers *)TIM2_BASE)
#define TIM3 ((Timer_Registers *)TIM3_BASE)
#define TIM4 ((Timer_Registers *)TIM4_BASE)
#define TIM5 ((Timer_Registers *)TIM5_BASE)

#endif