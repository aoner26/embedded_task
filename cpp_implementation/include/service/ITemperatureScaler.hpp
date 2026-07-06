/**
 * @file    ITemperatureScaler.hpp
 * @brief   Strategy interface for converting a raw ADC count into a
 *          physical temperature.
 *
 * Which implementation is used depends entirely on which sensor
 * revision the EEPROM says is fitted - see TemperatureScalerFactory.
 * TemperatureMonitor only ever sees this interface, never RevA/RevB
 * directly, which is the polymorphism at the heart of supporting both
 * hardware revisions from one code path.
 */

#ifndef SERVICE_ITEMPERATURESCALER_HPP
#define SERVICE_ITEMPERATURESCALER_HPP

#include <cstdint>

class ITemperatureScaler
{
public:
    virtual ~ITemperatureScaler() = default;

    /// Converts a raw ADC code into tenths of a degree Celsius.
    virtual std::int16_t toDeciCelsius(std::uint16_t rawAdcCount) const = 0;
};

#endif // SERVICE_ITEMPERATURESCALER_HPP
