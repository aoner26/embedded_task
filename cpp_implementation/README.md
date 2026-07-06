# Temperature Monitor - C++ (OOP) implementation

Same three layers as the C version (`app` -> `service` -> `driver`), expressed
with interfaces, polymorphism and constructor-based dependency injection
instead of function pointers and plain structs. See
`/docs/architecture/architecture.md` at the repository root, section 7, for
what specifically is different from the C version and why.

```
cpp_implementation/
├── include/
│   ├── driver/       IDigitalOutput, IAnalogInput, II2cBus, INonVolatileStorage,
│   │                 ITimer + their concrete Mcu*/Eeprom24LC implementations
│   ├── service/      DeviceConfigManager, ITemperatureScaler (+RevA/RevB),
│   │                 TemperatureMonitor, LedIndicator
│   └── app/          TemperatureMonitorApplication
├── src/              matching .cpp files
├── firmware/
│   ├── isr_vectors.cpp    TIM2_IRQHandler trampoline - compiled into the demo
│   ├── system_init.cpp/.hpp
│   └── startup/           vector table + linker script for the real target
│                          (NOT part of the PC build - see its README.md)
└── demo/
    └── main_pc_demo.cpp   PC test bench - the only file that constructs a
                           concrete driver class; everything above it is wired
                           together purely through interfaces
```

## Build & run

```sh
cd cpp_implementation
mkdir -p build && cd build
cmake .. -DCMAKE_CXX_COMPILER=g++   # or clang++, whichever has libstdc++/libc++ available
make
./temp_monitor_pc_demo
```

Requires a C++17 compiler and CMake >= 3.16. No external dependencies.

### Why "file not found" happens if you skip CMake

Headers are included relative to `include/` (and `firmware/` for the two
startup-support headers), e.g. `#include "app/TemperatureMonitorApplication.hpp"`
rather than a bare filename - `include/driver`, `include/service`, `include/app`
and `include/platform` each hold a different kind of file, and the directory
in the `#include` makes it unambiguous which one is meant at every call site.
That means the compiler needs `-I` flags pointing at `include/` and
`firmware/` - `cmake .. && make` sets these up for you via the
`target_include_directories(... PUBLIC include firmware)` lines in
`CMakeLists.txt`. Compiling a single file directly (e.g.
`g++ demo/main_pc_demo.cpp`) or opening just that file in an editor that
isn't aware of the CMake project will fail with something like
`app/TemperatureMonitorApplication.hpp: No such file or directory`.

**Without CMake**, everything can still be built in one shot from the
`cpp_implementation` directory:

```sh
g++ -std=c++17 -Iinclude -Ifirmware \
    src/driver/*.cpp src/service/*.cpp src/app/*.cpp \
    firmware/isr_vectors.cpp firmware/system_init.cpp \
    src/platform_sim/*.cpp \
    demo/main_pc_demo.cpp \
    -o temp_monitor_pc_demo
./temp_monitor_pc_demo
```

**If only your editor's IntelliSense is complaining** (red squiggles) while
`cmake && make` builds fine on the command line, regenerate with
`cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON` and point the editor's C/C++
extension at `build/compile_commands.json`.

## OOP concepts, and where to find them

| Concept | Where |
|---|---|
| Abstraction | `driver/I*.hpp` - services only ever see an interface, never a concrete MCU class |
| Polymorphism | `ITemperatureScaler` / `RevATemperatureScaler` / `RevBTemperatureScaler`, selected at runtime |
| Dependency injection | Every service/application constructor takes its collaborators by reference |
| Dependency inversion | `Eeprom24LC` depends on `II2cBus`, `DeviceConfigManager` depends on `INonVolatileStorage` - never on a concrete bus/device |
