/*
 * linker_script.ld
 *
 * Illustrative linker script for the target MCU used by this project
 * (256 KB flash / 64 KB SRAM Cortex-M part). Paired with startup_gcc.c,
 * this defines the _sidata/_sdata/_edata/_sbss/_ebss/_estack symbols
 * that the reset handler uses to initialize .data and .bss.
 *
 * Not used by the PC demonstration build - listed here for completeness,
 * the same way it would sit next to the startup file in the real
 * firmware project.
 */

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 256K
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 64K
}

ENTRY(Reset_Handler)

SECTIONS
{
    /* Vector table must be the very first thing in flash. */
    .isr_vector :
    {
        KEEP(*(.isr_vector))
    } > FLASH

    .text :
    {
        *(.text*)
        *(.rodata*)
    } > FLASH

    /* Record where the .data initializers end up in FLASH so
     * Reset_Handler knows where to copy them from. */
    _sidata = LOADADDR(.data);

    .data :
    {
        _sdata = .;
        *(.data*)
        _edata = .;
    } > RAM AT > FLASH

    .bss :
    {
        _sbss = .;
        *(.bss*)
        *(COMMON)
        _ebss = .;
    } > RAM

    ._user_heap_stack :
    {
        . = ALIGN(8);
        . = . + 0x400;   /* minimum heap  */
        . = . + 0x400;   /* minimum stack */
        . = ALIGN(8);
    } > RAM

    _estack = ORIGIN(RAM) + LENGTH(RAM);
}
