/**
 * @file    TemperatureMonitor.hpp
 * @brief   Reads the sensor, converts and classifies the temperature.
 *
 * This is where the +/-1 degC hysteresis (anti-flicker) logic lives.
 * Everything in onSampleTick() has to run in well under 100 us and must
 * never touch a bus - by the time it runs, the scale factor is already
 * cached and the sensor read is a simple register access.
 */

#ifndef SERVICE_TEMPERATUREMONITOR_HPP
#define SERVICE_TEMPERATUREMONITOR_HPP

#include <cstdint>
#include <memory>

#include "driver/IAnalogInput.hpp"
#include "service/ITemperatureScaler.hpp"

enum class TemperatureZone : std::uint8_t
{
    CriticalLow = 0,  ///< < 5 degC
    Normal,           ///< 5 .. 85 degC
    Warning,          ///< 85 .. 105 degC
    CriticalHigh      ///< >= 105 degC
};

class TemperatureMonitor
{
public:
    explicit TemperatureMonitor(IAnalogInput &sensor);

    /// Injects the scale factor to use for future samples. Must be
    /// called once - after the hardware revision has been read from the
    /// EEPROM - and before the first call to onSampleTick().
    void setScaler(std::unique_ptr<ITemperatureScaler> scaler);

    /// Reads one fresh sample and updates the classified zone.
    /// Intended to be called from the 100 us real-time task.
    void onSampleTick();

    TemperatureZone getZone() const;
    std::int16_t    getTemperatureDeciCelsius() const;

private:
    static TemperatureZone evaluateNextZone(TemperatureZone zone, std::int16_t temperatureDeciC);

    IAnalogInput                       &m_sensor;
    std::unique_ptr<ITemperatureScaler> m_scaler;

    // Written by the real-time task, read by whatever else wants to
    // know the current status (e.g. a display service) - volatile so
    // the compiler never hands out a cached copy across that boundary.
    volatile TemperatureZone m_currentZone           = TemperatureZone::Normal;
    volatile std::int16_t    m_lastTemperatureDeciC  = 0;
};

#endif // SERVICE_TEMPERATUREMONITOR_HPP
