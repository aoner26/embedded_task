/**
 * @file    TemperatureScalerFactory.hpp
 * @brief   Picks the right ITemperatureScaler for a given hardware revision.
 */

#ifndef SERVICE_TEMPERATURESCALERFACTORY_HPP
#define SERVICE_TEMPERATURESCALERFACTORY_HPP

#include <memory>

#include "service/DeviceConfig.hpp"
#include "service/ITemperatureScaler.hpp"

class TemperatureScalerFactory
{
public:
    static std::unique_ptr<ITemperatureScaler> create(HardwareRevision revision);
};

#endif // SERVICE_TEMPERATURESCALERFACTORY_HPP
