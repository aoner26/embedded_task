# Embedded Temperature Monitor

Bare-metal temperature monitoring and visualization device: an ADC-connected
sensor is sampled every 100 µs and three GPIO-connected LEDs (green / yellow /
red) show whether the reading is normal, at warning level, or critical. Board
configuration (which sensor revision is fitted, and the board's serial number)
lives in an I2C EEPROM.

This repository contains two independent implementations of the same design:

- **[`c_implementation/`](c_implementation/)** - plain C.
- **[`cpp_implementation/`](cpp_implementation/)** - C++, applying OOP
  (abstraction, polymorphism, dependency injection / inversion).

Both are demonstrated on a PC with the hardware (registers, ADC, I2C EEPROM)
mocked out, per the task's requirements - no real hardware or emulator is
needed. Architecture diagrams and write-up: **[`docs/architecture/`](docs/architecture/)**
- block diagram, UML component/activity/state-machine diagrams (both
implementations), and a UML class diagram for the C++ implementation.

## Requirements recap

- Temperature sensor on ADC, sampled every 100 µs, low jitter.
- 3 LEDs on GPIO: **G** normal (< 85 °C), **Y** warning (>= 85 °C), **R**
  critical (>= 105 °C or < 5 °C).
- Two hardware revisions with different sensor resolution:
  - Rev-A: 1 °C per ADC count.
  - Rev-B: 0.1 °C per ADC count.
- Hardware revision and serial number read from an I2C EEPROM.
- Layered design: application / service / driver (hardware abstraction).
- The EEPROM read is a one-time start-up task and must never run from inside
  the real-time (ISR-driven) sampling task.
- ±1 °C hysteresis on every LED transition, to avoid flicker from sensor
  noise sitting right on a threshold.

## Quick start

```sh
# C
cd c_implementation && mkdir -p build && cd build && cmake .. && make && ./temp_monitor_pc_demo

# C++
cd cpp_implementation && mkdir -p build && cd build && cmake .. -DCMAKE_CXX_COMPILER=g++ && make && ./temp_monitor_pc_demo
```

Each demo pre-programs a simulated EEPROM (as if it came from the factory),
brings the firmware up exactly the way it would on target, and then feeds a
simulated temperature sweep through the real sampling/hysteresis/LED path -
including dwelling right on the 5 °C / 85 °C / 105 °C boundaries, to show the
LEDs settling instead of flickering there.

## Repository layout

```
docs/architecture/          architecture.svg (diagram) + architecture.md (write-up)
c_implementation/           C version - see its README.md
cpp_implementation/         C++ / OOP version - see its README.md
```
