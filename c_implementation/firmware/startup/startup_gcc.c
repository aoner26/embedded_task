/**
 * @file    startup_gcc.c
 * @brief   Reset entry point and interrupt vector table for the target MCU.
 *
 * This is what actually runs first out of reset, before a single line
 * of main() executes. It is provided here for completeness /
 * documentation purposes, exactly as it would ship with the real
 * firmware - it is NOT part of the PC demonstration build (there is no
 * flash, no vector table and no linker at address 0x0 on a desktop OS).
 * See the top level README for how the two build targets are kept
 * separate.
 *
 * What the reset handler below does, in order:
 *   1. The core already loaded SP from vector[0] before this function
 *      was even entered, so the stack is valid on entry.
 *   2. Copy the .data section's initial values out of FLASH into RAM.
 *   3. Zero-fill the .bss section.
 *   4. Run global C++ constructors, if there are any (harmless no-op
 *      for the plain-C build of this project).
 *   5. Call SystemInit() to bring up the core clock tree.
 *   6. Call main(). If it ever returns, sit in an infinite loop instead
 *      of running off into whatever garbage follows in flash.
 */

#include <stdint.h>

/* Provided by the linker script (linker_script.ld). */
extern uint32_t _sidata;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;
extern uint32_t _estack;

extern int  main(void);
extern void SystemInit(void);
extern void __libc_init_array(void);

void Reset_Handler(void);
void Default_Handler(void);

/* Anything this firmware does not explicitly handle falls back to
 * Default_Handler through these weak aliases, so the link does not
 * fail just because some other exception could also fire. */
void NMI_Handler(void)        __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void MemManage_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void)   __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void)        __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler(void)   __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void)     __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void)    __attribute__((weak, alias("Default_Handler")));

/* The one application-specific interrupt this project needs: TIM2's
 * update event, which drives the 100 us sampling task. Implemented in
 * firmware/isr_vectors.c. */
extern void TIM2_IRQHandler(void);

/*
 * Cortex-M vector table. Kept in its own ".isr_vector" section so the
 * linker script can place it at address 0x00000000, which is where the
 * core looks for it straight out of reset.
 */
__attribute__((section(".isr_vector")))
void (* const g_pfnVectorTable[])(void) =
{
    (void (*)(void))&_estack,  /* 0: initial stack pointer */
    Reset_Handler,             /* 1: reset                 */
    NMI_Handler,               /* 2                        */
    HardFault_Handler,         /* 3                        */
    MemManage_Handler,         /* 4                        */
    BusFault_Handler,          /* 5                        */
    UsageFault_Handler,        /* 6                        */
    0,                         /* 7:  reserved             */
    0,                         /* 8:  reserved             */
    0,                         /* 9:  reserved             */
    0,                         /* 10: reserved             */
    SVC_Handler,               /* 11                       */
    DebugMon_Handler,          /* 12                       */
    0,                         /* 13: reserved             */
    PendSV_Handler,            /* 14                       */
    SysTick_Handler,           /* 15                       */

    /* --- external (peripheral) interrupts, IRQ0 upward --- */
    TIM2_IRQHandler,           /* IRQ0: 100 us sample tick */
};

void Reset_Handler(void)
{
    uint32_t *src;
    uint32_t *dst;

    /* .data lives in RAM at runtime but its initial values only exist
     * in FLASH - copy them across before anything touches a global
     * with a non-zero initializer. */
    src = &_sidata;
    dst = &_sdata;
    while (dst < &_edata)
    {
        *dst++ = *src++;
    }

    /* The C standard guarantees zero-initialized statics read as zero
     * - make that true by clearing .bss ourselves, since RAM does not
     * come out of reset pre-zeroed. */
    dst = &_sbss;
    while (dst < &_ebss)
    {
        *dst++ = 0u;
    }

    __libc_init_array();

    SystemInit();

    (void)main();

    while (1)
    {
        /* main() must never return on a bare-metal target. */
    }
}

void Default_Handler(void)
{
    while (1)
    {
        /* Caught an interrupt this firmware does not know how to
         * service - stop here so it is obvious during bring-up rather
         * than silently falling through to whatever handler happens to
         * follow it in memory. */
    }
}
