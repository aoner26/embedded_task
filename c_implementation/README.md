# Temperature Monitor - C implementation

Layered as `app` -> `service` -> `driver`, with a mocked hardware layer
(`platform_sim/`) standing in for the real registers so the whole thing
builds and runs on a normal PC. See `/docs/architecture/architecture.md`
at the repository root for the design rationale.

```
c_implementation/
├── include/                driver / service / app / platform headers
├── src/
│   ├── driver/              gpio, adc, i2c, eeprom, timer drivers
│   ├── service/             device config, temperature, LED indicator
│   ├── app/                 start-up sequencing + the real-time task
│   └── platform_sim/        RAM-backed mock registers + mock I2C/ADC "hardware"
├── firmware/
│   ├── isr_vectors.c         TIM2_IRQHandler - compiled into the demo
│   ├── system_init.c/.h      clock bring-up placeholder
│   └── startup/              vector table + linker script for the real target
│                             (NOT part of the PC build - see its README.md)
└── demo/
    └── main_pc_demo.c        PC test bench: programs the mock EEPROM, sweeps a
                              simulated temperature profile through every zone
```

## Build & run

```sh
cd c_implementation
mkdir -p build && cd build
cmake ..
make
./temp_monitor_pc_demo
```

Requires a C11 compiler and CMake >= 3.16. No external dependencies.

### Why "file not found" happens if you skip CMake

The headers are included with their path relative to `include/` (and
`firmware/` for the two startup-support headers), e.g.
`#include "app/app_temp_monitor.h"` rather than `#include "app_temp_monitor.h"`.
That is deliberate - `include/driver`, `include/service`, `include/app` and
`include/platform` each contain files with different purposes, and the
directory in the `#include` makes it unambiguous which one you're pulling in
at every call site.

It does mean the compiler has to be told where `include/` and `firmware/`
are with `-I` flags - `cmake .. && make` does this for you automatically
(see the `target_include_directories(... PUBLIC include firmware)` lines in
`CMakeLists.txt`). If instead you compile a single file directly, e.g.
`gcc demo/main_pc_demo.c`, or open just that one file in an editor/IDE that
doesn't know about the CMake project, the compiler has no way to know where
`include/` is and fails with `app/app_temp_monitor.h: No such file or
directory`.

**If you don't want to use CMake**, compile everything in one shot with the
include paths given explicitly, from the `c_implementation` directory:

```sh
gcc -std=c11 -Iinclude -Ifirmware \
    src/driver/*.c src/service/*.c src/app/*.c \
    firmware/isr_vectors.c firmware/system_init.c \
    src/platform_sim/*.c \
    demo/main_pc_demo.c \
    -o temp_monitor_pc_demo
./temp_monitor_pc_demo
```

**If your editor is underlining the `#include` lines red** (VS Code, CLion,
etc.) even though `cmake && make` builds fine on the command line, that's an
IntelliSense/indexer configuration issue, not a real compile error - the
indexer isn't reading the CMake include paths. Regenerate the build with
`cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON` and point your editor's C/C++
extension at the resulting `build/compile_commands.json` (VS Code's C/C++
extension and CLion both pick this up automatically if it's in - or symlinked
into - the project root).

## What the demo shows

- The one-time EEPROM read (`cfg_svc_load_from_eeprom`) happening before the
  sampling timer is armed.
- Every 100 µs tick (`app_temp_monitor_on_sample_tick`, called directly by the
  demo the same way `TIM2_IRQHandler` would call it on target) reading a raw ADC
  sample, converting it and running it through the hysteresis state machine.
- The three LEDs settling instead of flickering as the simulated temperature
  loiters right on top of the 5 °C / 85 °C / 105 °C boundaries.
