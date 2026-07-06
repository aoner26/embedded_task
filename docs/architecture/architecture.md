# Architecture - Temperature Monitor

This folder contains the block diagram plus a set of UML diagrams covering the
areas an interviewer would typically expect to see modeled explicitly:

| Diagram | File | Notes |
|---|---|---|
| Layered block diagram | `architecture.svg` | The informal, high-level version of the same layering, referenced throughout this document. |
| Component diagram | `component_diagram.svg` | Applies to both implementations. Components, provided interfaces (lollipops) and the "Target Hardware" vs. "PC Demonstration Build" package split. |
| Activity diagram | `activity_diagram.svg` | Swimlaned (Application / Service / Driver / Hardware) sequence for both the one-time start-up and the repeating 100 µs sampling task. |
| State machine diagram | `temperature_state_machine.svg` | The `evaluate_next_zone()` / `TemperatureMonitor::evaluateNextZone()` hysteresis state machine, with every guard condition labelled. |
| Class diagram (C++ only) | `cpp_class_diagram.svg` | Every interface, concrete driver, service and application class in `cpp_implementation/`, with realization/dependency/association/composition relationships. Laid out on a fixed per-subsystem column grid so relationship lines never cross another class. |

`diagram_src/` contains the small Python script (and a shared layout helper,
`umllib.py`) used to generate the four UML diagrams - see its `README.md` to
regenerate them after an edit.

## 1. Overview

The device reads a temperature sensor over ADC every 100 µs and drives one of three
status LEDs (green / yellow / red) via GPIO depending on which of three ranges the
temperature falls into. Two hardware revisions exist with sensors of different
resolution, and the board's revision plus its serial number live in an on-board
EEPROM, accessed over I2C.

The same design is implemented twice, once in C and once in C++. Both follow the
same three layers:

```
Application  -> orchestrates start-up and the real-time task
Service      -> business logic (scaling, hysteresis, LED mapping, config read-out)
Driver / HAL -> talks to registers/peripherals, nothing else
```

The driver layer is the only place that ever touches a register. The service layer
never includes a register header, and the application layer never includes a driver
header directly in the C++ version - it only sees interfaces.

## 2. Why the EEPROM read happens once, outside the ISR

I2C is a shared, clocked, multi-byte bus protocol. Even in the best case a single
transaction takes tens of microseconds, and the exact timing depends on bus
arbitration, clock stretching, etc. - none of that is compatible with "sample the
sensor every 100 µs with very low jitter." On top of that, the hardware revision and
the serial number are both static: they are set once when the board is manufactured
and never change again while it's running, so there's no reason to read them more
than once either way.

Because of that, both implementations read the EEPROM exactly once, during start-up,
strictly before the sampling timer is armed:

- C: `app_temp_monitor_init()` calls `cfg_svc_load_from_eeprom()`, then
  `temp_svc_init()` caches the resulting scale factor. Only after that does it call
  `tmr_drv_configure_periodic_us()`. The timer is only actually started afterwards
  by a separate `app_temp_monitor_start()` call.
- C++: `TemperatureMonitorApplication::initialize()` calls
  `DeviceConfigManager::loadOnce()`, builds the matching `ITemperatureScaler` through
  `TemperatureScalerFactory`, and injects it into `TemperatureMonitor` before
  `ITimer::configurePeriodicMicros()` is even called. `start()` is a separate step.

From that point on, the periodic task (`app_temp_monitor_on_sample_tick()` /
`TemperatureMonitorApplication::onRealTimeTick()`) only ever touches the ADC
register and a couple of GPIO pins - no bus transaction, no blocking call, nothing
with variable timing.

## 3. Hysteresis (why the LEDs don't flicker)

Right at a threshold, sensor/ADC noise of even a fraction of a degree would make a
naive "if temp >= 85, turn on yellow" check flip back and forth every sample. Both
implementations instead run a small state machine that remembers which zone it was
last in and only crosses a boundary in the direction that got it there once the
reading is a full degree past it:

| Boundary | Enters when | Only leaves when |
|---|---|---|
| Normal -> Warning | temp >= 85.0 °C | temp < 84.0 °C |
| Warning -> Critical (high) | temp >= 105.0 °C | temp < 104.0 °C |
| Normal -> Critical (low) | temp < 5.0 °C | temp >= 6.0 °C |

This is the `evaluate_next_zone()` / `TemperatureMonitor::evaluateNextZone()`
function - a pure function of `(current zone, new reading)` with no side effects,
which is what makes it possible to reason about (and unit test) without touching any
hardware. See `temperature_state_machine.svg` for the full state diagram.

The same ±1 °C tolerance constant is applied identically to **all three**
boundaries (5.0 / 85.0 / 105.0 °C) and to **both** directions of crossing each
one - the diagram's boundary table spells this out explicitly. It's just that
the *entry* edge of a boundary is pinned to the exact number the requirements
give ("warning ≥ 85 °C", etc. - so its offset from that nominal value is
0 °C by construction, since shifting it would either delay a safety-relevant
indication or directly contradict the requirement), while the *recovery* edge
is where a reading sitting still would otherwise flicker the LED, so that is
where the full 1.0 °C is actually subtracted (or added, on the low side).
Both edges of every boundary are derived from the same `nominal ± tolerance`
formula - the shift is just algebraically zero on one side of each boundary.

All temperature values inside the sampling path are represented as integers in
tenths of a degree Celsius rather than as `float`/`double`. That sidesteps rounding
surprises in the threshold comparisons and, more importantly, keeps the 100 µs path
free of floating point entirely, which matters if this ever runs on a Cortex-M0
part without an FPU.

## 4. Rev-A / Rev-B sensor support

- Rev-A: 1 raw ADC count = 1.0 °C
- Rev-B: 1 raw ADC count = 0.1 °C

In C this is a single multiplier (`deci_c_per_adc_count`, either 10 or 1) cached by
`temp_svc_init()` after the EEPROM read. In C++ it's expressed as a strategy: two
`ITemperatureScaler` implementations (`RevATemperatureScaler`, `RevBTemperatureScaler`)
and a `TemperatureScalerFactory` that picks one based on the `HardwareRevision` read
out of the EEPROM. `TemperatureMonitor` only ever calls `m_scaler->toDeciCelsius()` -
it has no idea which revision it's running against.

## 5. `volatile` usage

Both `TemperatureMonitor`'s current zone/temperature and their C equivalents
(`current_zone`, `last_temperature_deci_c` in `temperature_service.c`) are declared
`volatile`. They are written from the real-time sampling task and may be read from
other, non-real-time code (a display, a logger, the demo harness) - `volatile` stops
the compiler from caching a stale value across that boundary or reordering the
update away from where the ISR actually performs it. Everything else (thresholds,
scale factors, pin numbers) is fixed at start-up and does not need it.

## 6. Memory initialization / vector table

`c_implementation/firmware/startup/` and `cpp_implementation/firmware/startup/`
contain `startup_gcc.c[pp]` and `linker_script.ld` for the real target: the
Cortex-M-style vector table, `Reset_Handler` (copies `.data` out of flash, zeroes
`.bss`, and - in the C++ build - calls `__libc_init_array()` to run global object
constructors before `main()`), and a matching linker script defining the FLASH/RAM
memory map. Neither is compiled into the PC demo (there is no flash or address
`0x00000000` on a desktop OS to place them at); they're included so the bring-up
this firmware would need on real silicon is fully shown, not hand-waved. See the
`README.md` next to each for exactly why they're excluded from that build.

The one interrupt this project actually needs, `TIM2_IRQHandler`, is implemented in
`firmware/isr_vectors.c[pp]` and *is* compiled into the demo - it's real handler
logic, not target-specific boot plumbing, it's simply never triggered by a real
timer on a PC (as allowed by the task).

## 7. Where the C++ version differs (OOP)

- **Abstraction**: every driver is exposed to the layer above it only through an
  interface (`IDigitalOutput`, `IAnalogInput`, `II2cBus`, `INonVolatileStorage`,
  `ITimer`) - the services and the application never see `McuGpioOutput` or
  `McuI2cBus` by name.
- **Polymorphism**: `ITemperatureScaler` has two concrete implementations, chosen at
  runtime from the EEPROM's hardware revision byte, and `TemperatureMonitor` calls
  the interface without a single `if`/`switch` on which one it got.
- **Dependency injection**: every service and the application take their
  collaborators as constructor references (`TemperatureMonitor(IAnalogInput&)`,
  `LedIndicator(IDigitalOutput&, IDigitalOutput&, IDigitalOutput&)`,
  `TemperatureMonitorApplication(TemperatureMonitor&, LedIndicator&,
  DeviceConfigManager&, ITimer&)`). Nothing above `main()` constructs a concrete
  driver itself.
- **Dependency inversion**: `Eeprom24LC` (a device driver) depends on `II2cBus` (an
  abstraction owned by the driver layer, not the bus implementation), and
  `DeviceConfigManager` (a service) depends on `INonVolatileStorage`, not on
  `Eeprom24LC` or I2C at all - the service layer would not need to change if the
  configuration moved to SPI flash tomorrow.
- The EEPROM-driven scaler selection is a good example of *late* dependency
  injection: `TemperatureMonitor` is constructed with a safe default scaler, and the
  real one is only injected once `TemperatureMonitorApplication::initialize()` has
  actually read the EEPROM - the dependency isn't known until runtime configuration
  says which one it is.

See `cpp_class_diagram.svg` for the full picture: every interface, every concrete
class that realizes one, and every constructor-injected relationship listed above,
drawn out with attributes and operations.
