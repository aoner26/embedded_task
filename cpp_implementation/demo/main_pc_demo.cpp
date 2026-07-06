/**
 * @file    main_pc_demo.cpp
 * @brief   PC-hosted demonstration of the temperature monitor firmware
 *          (C++ / OOP implementation).
 *
 * Plays the role of everything that would normally be soldered to the
 * board: pre-programs the simulated EEPROM the way a factory programmer
 * would, then drives the simulated ADC through a temperature profile
 * the way a heat gun and a cold spray can would on a real bench.
 *
 * Note how this file is the only place in the whole program that
 * constructs a concrete driver class - everything above it
 * (TemperatureMonitor, LedIndicator, DeviceConfigManager,
 * TemperatureMonitorApplication) is wired together here purely through
 * interfaces (dependency injection), so swapping McuGpioOutput for some
 * other IDigitalOutput would not touch a single line of service or
 * application code.
 */

#include <array>
#include <cstdint>
#include <cstdio>
#include <string>

#include "app/TemperatureMonitorApplication.hpp"
#include "driver/Eeprom24LC.hpp"
#include "driver/McuAdcChannel.hpp"
#include "driver/McuGpioOutput.hpp"
#include "driver/McuI2cBus.hpp"
#include "driver/McuTimer.hpp"
#include "platform/mock_environment.hpp"
#include "service/DeviceConfigManager.hpp"
#include "service/LedIndicator.hpp"
#include "service/TemperatureMonitor.hpp"

namespace
{

// Pin/channel assignment for this board.
constexpr std::uint8_t kLedPinGreen  = 5; // PA5
constexpr std::uint8_t kLedPinYellow = 6; // PA6
constexpr std::uint8_t kLedPinRed    = 7; // PA7
constexpr std::uint8_t kAdcChannel   = 0; // PA0

// What the EEPROM would already contain by the time the board is
// powered on for the very first time.
constexpr HardwareRevision kDemoHwRevision   = HardwareRevision::RevB;
constexpr const char      *kDemoSerialNumber = "ABC1234";

std::uint16_t deciCelsiusToRawCount(std::int16_t deciCelsius)
{
    // The test bench has to know the sensor's real transfer curve to
    // generate believable ADC codes - this mirrors the exact scale
    // factor RevBTemperatureScaler/RevATemperatureScaler apply.
    return (kDemoHwRevision == HardwareRevision::RevB)
               ? static_cast<std::uint16_t>(deciCelsius)
               : static_cast<std::uint16_t>(deciCelsius / 10);
}

const char *zoneToString(TemperatureZone zone)
{
    switch (zone)
    {
        case TemperatureZone::CriticalLow:  return "RED    - critical (low)";
        case TemperatureZone::Normal:       return "GREEN  - ok";
        case TemperatureZone::Warning:      return "YELLOW - warning";
        case TemperatureZone::CriticalHigh: return "RED    - critical (high)";
        default:                            return "UNKNOWN";
    }
}

} // namespace

int main()
{
    // --- Program the simulated EEPROM, as the factory would ------------
    const auto revisionByte = static_cast<std::uint8_t>(kDemoHwRevision);
    mcu::mockEepromProgram(0x00, &revisionByte, 1);
    mcu::mockEepromProgram(0x01,
                            reinterpret_cast<const std::uint8_t *>(kDemoSerialNumber),
                            std::string(kDemoSerialNumber).size() + 1); // + '\0'

    std::printf("=== Temperature Monitor - PC demonstration (C++ / OOP implementation) ===\n\n");

    // --- Concrete hardware (mocked) -------------------------------------
    McuGpioOutput ledGreen(kLedPinGreen);
    McuGpioOutput ledYellow(kLedPinYellow);
    McuGpioOutput ledRed(kLedPinRed);
    McuAdcChannel adcChannel(kAdcChannel);
    McuI2cBus     i2cBus;
    Eeprom24LC    eeprom(i2cBus);
    McuTimer      samplingTimer;

    ledGreen.configure();
    ledYellow.configure();
    ledRed.configure();
    adcChannel.enable();
    i2cBus.enable();

    // --- Services, depending only on the interfaces above ---------------
    TemperatureMonitor  temperatureMonitor(adcChannel);
    LedIndicator        ledIndicator(ledGreen, ledYellow, ledRed);
    DeviceConfigManager configManager(eeprom);

    // --- Application, wired together purely through references ----------
    TemperatureMonitorApplication application(temperatureMonitor, ledIndicator, configManager, samplingTimer);
    application.registerAsActiveInstance();

    if (!application.initialize())
    {
        std::fprintf(stderr, "FATAL: failed to read board configuration from EEPROM\n");
        return 1;
    }

    const DeviceConfig &config = application.boardConfig();
    std::printf("Hardware revision : %s\n",
                (config.hwRevision == HardwareRevision::RevB) ? "Rev-B (0.1 degC/count)" : "Rev-A (1.0 degC/count)");
    std::printf("Serial number     : %s\n\n", config.serialNumber.data());

    // Only now, with the configuration cached, is it safe to let the
    // real-time task run.
    application.start();

    /*
     * Simulated temperature profile, in tenths of a degree Celsius.
     * Sweeps through every zone and deliberately loiters right on top
     * of the 5.0 / 85.0 / 105.0 degC boundaries to demonstrate that the
     * +/-1 degC hysteresis band keeps the LEDs from chattering there.
     * Each entry stands in for one 100 us tick of the real-time task.
     */
    static const std::array<std::int16_t, 40> kTemperatureProfileDeciC = {{
        200, 400, 600, 800,           // normal, climbing
        840, 845, 849,                // nibbling the 85.0 degC edge
        850, 860, 900, 950, 1000,     // into warning
        1040, 1045, 1049,             // nibbling the 105.0 degC edge
        1050, 1080, 1100,             // into critical - high
        1049, 1041, 1035,             // dropping back into the hysteresis band - must stay RED
        1000, 900, 840,               // back through warning
        830, 800, 600, 400,           // back to normal
        100, 60, 51,                  // nibbling the 5.0 degC edge
        49, 40, 10,                   // into critical - low
        51, 55, 60,                   // rising back into the hysteresis band - must stay RED
        100, 300, 500                 // recovered to normal
    }};

    std::printf("%-10s %-9s %-10s %-24s\n", "tick", "raw_adc", "temp_C", "led_status");
    std::printf("--------------------------------------------------------\n");

    TemperatureZone previousZone = temperatureMonitor.getZone();

    for (std::size_t tick = 0; tick < kTemperatureProfileDeciC.size(); ++tick)
    {
        const std::int16_t  deciCelsius = kTemperatureProfileDeciC[tick];
        const std::uint16_t rawCount    = deciCelsiusToRawCount(deciCelsius);

        // Stand in for the sensor driving a new voltage onto the ADC
        // pin, then run exactly the routine TIM2_IRQHandler() would run
        // on target every 100 us.
        mcu::mockAdcSetRawSample(rawCount);
        application.onRealTimeTick();

        const TemperatureZone zone            = temperatureMonitor.getZone();
        const std::int16_t    reportedDeciC   = temperatureMonitor.getTemperatureDeciCelsius();
        const std::int16_t    wholeDegrees    = static_cast<std::int16_t>(reportedDeciC / 10);
        const std::int16_t    tenthDegrees    = static_cast<std::int16_t>((reportedDeciC < 0 ? -reportedDeciC : reportedDeciC) % 10);

        std::printf("%-10zu %-9u %+6d.%01d C  %-24s%s\n",
                    tick, rawCount,
                    wholeDegrees, tenthDegrees,
                    zoneToString(zone),
                    (zone != previousZone) ? "  <-- LED transition" : "");

        previousZone = zone;
    }

    std::printf("\nDemo finished - %zu samples processed.\n", kTemperatureProfileDeciC.size());
    return 0;
}
