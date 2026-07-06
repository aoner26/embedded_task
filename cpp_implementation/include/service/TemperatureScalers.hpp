/**
 * @file    TemperatureScalers.hpp
 * @brief   Concrete ITemperatureScaler implementations, one per sensor revision.
 */

#ifndef SERVICE_TEMPERATURESCALERS_HPP
#define SERVICE_TEMPERATURESCALERS_HPP

#include "service/ITemperatureScaler.hpp"

/// Rev-A sensor: 1.0 degC per ADC count (e.g. a raw reading of 10 means 10 degC).
class RevATemperatureScaler final : public ITemperatureScaler
{
public:
    std::int16_t toDeciCelsius(std::uint16_t rawAdcCount) const override;
};

/// Rev-B sensor: 0.1 degC per ADC count (e.g. a raw reading of 100 means 10 degC).
class RevBTemperatureScaler final : public ITemperatureScaler
{
public:
    std::int16_t toDeciCelsius(std::uint16_t rawAdcCount) const override;
};

#endif // SERVICE_TEMPERATURESCALERS_HPP
