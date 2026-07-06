#include "service/TemperatureScalers.hpp"

std::int16_t RevATemperatureScaler::toDeciCelsius(std::uint16_t rawAdcCount) const
{
    return static_cast<std::int16_t>(static_cast<std::int32_t>(rawAdcCount) * 10);
}

std::int16_t RevBTemperatureScaler::toDeciCelsius(std::uint16_t rawAdcCount) const
{
    return static_cast<std::int16_t>(rawAdcCount);
}
