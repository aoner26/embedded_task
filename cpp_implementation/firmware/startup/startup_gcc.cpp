/**
 * @file    startup_gcc.cpp
 * @brief   Reset entry point and interrupt vector table for the target MCU.
 *
 * This is what actually runs first out of reset, before a single line
 * of main() executes. It is provided for completeness/documentation,
 * exactly as it would ship with the real firmware - it is NOT part of
 * the PC demonstration build (there is no flash, no vector table and no
 * linker at address 0x0 on a desktop OS). See the top level README for
 * how the two build targets are kept separate.
 *
 * What the reset handler below does, in order:
 *   1. The core already loaded SP from vector[0] before this function
 *      was even entered, so the stack is valid on entry.
 *   2. Copy the .data section's initial values out of FLASH into RAM.
 *   3. Zero-fill the .bss section.
 *   4. Call __libc_init_array(), which runs every C++ static/global
 *      object's constructor (this step does not exist in the C version
 *      of this project - it is the one piece of boot plumbing that is
 *      genuinely specific to the C++ build, since global objects with
 *      non-trivial constructors need to run before main() can rely on
 *      them being valid).
 *   5. Call SystemInit() to bring up the core clock tree.
 *   6. Call main(). If it ever returns, sit in an infinite loop instead
 *      of running off into whatever garbage follows in flash.
 */

#include <cstdint>

extern "C"
{
// Provided by the linker script (linker_script.ld).
extern std::uint32_t _sidata;
extern std::uint32_t _sdata;
extern std::uint32_t _edata;
extern std::uint32_t _sbss;
extern std::uint32_t _ebss;
extern std::uint32_t _estack;

extern int  main(void);
extern void SystemInit(void);
extern void __libc_init_array(void);

void Reset_Handler(void);
void Default_Handler(void);

void NMI_Handler(void)        __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void MemManage_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void)   __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void)        __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler(void)   __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void)     __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void)    __attribute__((weak, alias("Default_Handler")));

// The one application-specific interrupt this project needs: TIM2's
// update event, which drives the 100 us sampling task. Implemented in
// firmware/isr_vectors.cpp.
void TIM2_IRQHandler(void);
} // extern "C"

/*
 * Cortex-M vector table. Kept in its own ".isr_vector" section so the
 * linker script can place it at address 0x00000000, which is where the
 * core looks for it straight out of reset.
 */
extern "C" __attribute__((section(".isr_vector")))
void (* const g_pfnVectorTable[])(void) =
{
    reinterpret_cast<void (*)(void)>(&_estack), // 0: initial stack pointer
    Reset_Handler,                              // 1: reset
    NMI_Handler,                                // 2
    HardFault_Handler,                          // 3
    MemManage_Handler,                          // 4
    BusFault_Handler,                           // 5
    UsageFault_Handler,                         // 6
    nullptr,                                    // 7:  reserved
    nullptr,                                    // 8:  reserved
    nullptr,                                    // 9:  reserved
    nullptr,                                    // 10: reserved
    SVC_Handler,                                // 11
    DebugMon_Handler,                           // 12
    nullptr,                                    // 13: reserved
    PendSV_Handler,                             // 14
    SysTick_Handler,                            // 15

    // --- external (peripheral) interrupts, IRQ0 upward ---
    TIM2_IRQHandler,                            // IRQ0: 100 us sample tick
};

extern "C" void Reset_Handler(void)
{
    std::uint32_t *src;
    std::uint32_t *dst;

    // .data lives in RAM at runtime but its initial values only exist in
    // FLASH - copy them across before anything touches a global with a
    // non-zero initializer.
    src = &_sidata;
    dst = &_sdata;
    while (dst < &_edata)
    {
        *dst++ = *src++;
    }

    // RAM does not come out of reset pre-zeroed - clear .bss ourselves
    // so zero-initialized statics actually read as zero.
    dst = &_sbss;
    while (dst < &_ebss)
    {
        *dst++ = 0u;
    }

    // Runs the constructor of every global/static object with one -
    // required before main() (or anything it calls) can safely use
    // objects like the driver/service instances constructed at file
    // scope.
    __libc_init_array();

    SystemInit();

    (void)main();

    while (true)
    {
        // main() must never return on a bare-metal target.
    }
}

extern "C" void Default_Handler(void)
{
    while (true)
    {
        // Caught an interrupt this firmware does not know how to
        // service - stop here so it is obvious during bring-up rather
        // than silently falling through to whatever handler happens to
        // follow it in memory.
    }
}
