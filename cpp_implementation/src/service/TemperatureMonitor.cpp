#include "service/TemperatureMonitor.hpp"
#include "service/TemperatureScalers.hpp"

#include <utility>

namespace
{
// Thresholds from the spec, expressed in tenths of a degree so the
// comparisons below stay pure integer arithmetic.
constexpr std::int16_t kWarningThresholdDeciC      = 850;   //  85.0 degC
constexpr std::int16_t kCriticalHighThresholdDeciC = 1050;  // 105.0 degC
constexpr std::int16_t kCriticalLowThresholdDeciC  = 50;    //   5.0 degC

// +/-1 degC of hysteresis on every transition, per the requirement -
// keeps a reading sitting right on a boundary from making the LEDs
// flicker back and forth.
constexpr std::int16_t kHysteresisToleranceDeciC = 10;
} // namespace

TemperatureMonitor::TemperatureMonitor(IAnalogInput &sensor)
    : m_sensor(sensor)
    , m_scaler(std::make_unique<RevATemperatureScaler>()) // safe default until setScaler() runs
{
}

void TemperatureMonitor::setScaler(std::unique_ptr<ITemperatureScaler> scaler)
{
    m_scaler = std::move(scaler);
}

TemperatureZone TemperatureMonitor::evaluateNextZone(TemperatureZone zone, std::int16_t temperatureDeciC)
{
    switch (zone)
    {
        case TemperatureZone::CriticalLow:
            if (temperatureDeciC >= (kCriticalLowThresholdDeciC + kHysteresisToleranceDeciC))
            {
                return TemperatureZone::Normal;
            }
            return TemperatureZone::CriticalLow;

        case TemperatureZone::Warning:
            if (temperatureDeciC >= kCriticalHighThresholdDeciC)
            {
                return TemperatureZone::CriticalHigh;
            }
            if (temperatureDeciC < (kWarningThresholdDeciC - kHysteresisToleranceDeciC))
            {
                return TemperatureZone::Normal;
            }
            return TemperatureZone::Warning;

        case TemperatureZone::CriticalHigh:
            if (temperatureDeciC < (kCriticalHighThresholdDeciC - kHysteresisToleranceDeciC))
            {
                return TemperatureZone::Warning;
            }
            return TemperatureZone::CriticalHigh;

        case TemperatureZone::Normal:
        default:
            if (temperatureDeciC >= kWarningThresholdDeciC)
            {
                return TemperatureZone::Warning;
            }
            if (temperatureDeciC < kCriticalLowThresholdDeciC)
            {
                return TemperatureZone::CriticalLow;
            }
            return TemperatureZone::Normal;
    }
}

void TemperatureMonitor::onSampleTick()
{
    const std::uint16_t rawSample      = m_sensor.readRaw();
    const std::int16_t  temperatureDeciC = m_scaler->toDeciCelsius(rawSample);

    m_currentZone = evaluateNextZone(m_currentZone, temperatureDeciC);
    m_lastTemperatureDeciC = temperatureDeciC;
}

TemperatureZone TemperatureMonitor::getZone() const
{
    return m_currentZone;
}

std::int16_t TemperatureMonitor::getTemperatureDeciCelsius() const
{
    return m_lastTemperatureDeciC;
}
