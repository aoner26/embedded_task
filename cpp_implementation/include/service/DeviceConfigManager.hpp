/**
 * @file    DeviceConfigManager.hpp
 * @brief   One-time board configuration read-out.
 *
 * The hardware revision and serial number never change while the board
 * is running - they are read exactly once, at boot, before the
 * real-time sampling timer is ever armed. See
 * TemperatureMonitorApplication::initialize() for why this must never
 * be called from anywhere near the 100 us periodic task.
 */

#ifndef SERVICE_DEVICECONFIGMANAGER_HPP
#define SERVICE_DEVICECONFIGMANAGER_HPP

#include "driver/INonVolatileStorage.hpp"
#include "service/DeviceConfig.hpp"

class DeviceConfigManager
{
public:
    explicit DeviceConfigManager(INonVolatileStorage &storage);

    /// Blocking read of the board configuration.
    /// @return true if the storage answered and outConfig was populated.
    bool loadOnce(DeviceConfig &outConfig);

private:
    INonVolatileStorage &m_storage;
};

#endif // SERVICE_DEVICECONFIGMANAGER_HPP
