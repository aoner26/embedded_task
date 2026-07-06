#include "service/TemperatureScalerFactory.hpp"
#include "service/TemperatureScalers.hpp"

std::unique_ptr<ITemperatureScaler> TemperatureScalerFactory::create(HardwareRevision revision)
{
    if (revision == HardwareRevision::RevB)
    {
        return std::make_unique<RevBTemperatureScaler>();
    }

    return std::make_unique<RevATemperatureScaler>();
}
