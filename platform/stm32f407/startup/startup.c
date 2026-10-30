#include <stdint.h>

extern uint32_t _sidata;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;
extern uint32_t _estack;

extern int main(void);

void Default_Handler(void)
{
    while (1)
        ;
}

void Reset_Handler(void)
{
    uint32_t *src = &_sidata;
    uint32_t *dest = &_sdata;

    /* Copy initialized global/static variables from Flash to RAM. */
    while (dest != &_edata)
    {
        *dest++ = *src++;
    }

    /* Zero uninitialized global/static variables in RAM. */
    dest = &_sbss;

    while (dest != &_ebss)
    {
        *dest++ = 0U;
    }

    (void)main();

    /* There is no operating system to return to. */
    while (1)
    {
    }
}

__attribute__((section(".isr_vector")))
const uint32_t vector_table[98] =
    {
        [0] = (uintptr_t)&_estack,
        [1] = (uint32_t)Reset_Handler,
        [2] = (uint32_t)Default_Handler,  /* NMI */
        [3] = (uint32_t)Default_Handler,  /* HardFault */
        [4] = (uint32_t)Default_Handler,  /* MemManage */
        [5] = (uint32_t)Default_Handler,  /* BusFault */
        [6] = (uint32_t)Default_Handler,  /* UsageFault */
        [7] = 0,                          /* Reserved */
        [8] = 0,                          /* Reserved */
        [9] = 0,                          /* Reserved */
        [10] = 0,                         /* Reserved */
        [11] = (uint32_t)Default_Handler, /* SVCall */
        [12] = (uint32_t)Default_Handler, /* DebugMonitor */
        [13] = 0,                         /* Reserved */
        [14] = (uint32_t)Default_Handler, /* PendSV */
        [15] = (uint32_t)Default_Handler, /* SysTick */

        [16 ... 97] = (uint32_t)Default_Handler /* IRQ */

};
