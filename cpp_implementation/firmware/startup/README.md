# Target startup code

`startup_gcc.cpp` and `linker_script.ld` are what would actually place
this firmware in flash and get it running on the real MCU (vector
table, `Reset_Handler`, `.data`/`.bss` initialization, and - specific to
this C++ build - running global object constructors via
`__libc_init_array()` before `main()`). They are included for
completeness/documentation, matching what the real firmware repository
would contain next to the driver/service/app sources.

They are **not** part of the PC demonstration build in `CMakeLists.txt`:
there is no flash, no address `0x00000000` vector table and no linker
script on a desktop OS, so building them would require a real
`arm-none-eabi-g++` cross toolchain and a target board (or at least an
instruction-set simulator such as QEMU) to mean anything - neither of
which this exercise calls for. `firmware/isr_vectors.cpp`, on the other
hand, *is* compiled into the demo, since it contains the actual
interrupt handler logic (`TIM2_IRQHandler`) rather than target-specific
boot plumbing.
