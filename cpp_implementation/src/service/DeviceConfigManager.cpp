#include "service/DeviceConfigManager.hpp"

namespace
{
// EEPROM memory map - this is the one place in the whole project that
// needs to know where things live in the configuration EEPROM.
constexpr std::uint8_t kOffsetHwRevision   = 0x00;
constexpr std::uint8_t kOffsetSerialNumber = 0x01;
constexpr std::size_t  kSerialNumberStoredLen = kSerialNumberMaxLen - 1;
} // namespace

DeviceConfigManager::DeviceConfigManager(INonVolatileStorage &storage)
    : m_storage(storage)
{
}

bool DeviceConfigManager::loadOnce(DeviceConfig &outConfig)
{
    std::uint8_t rawRevisionByte = 0;
    if (!m_storage.readBlock(kOffsetHwRevision, &rawRevisionByte, 1u))
    {
        return false;
    }

    DeviceConfig loadedConfig;
    loadedConfig.hwRevision = (rawRevisionByte == static_cast<std::uint8_t>(HardwareRevision::RevB))
                                  ? HardwareRevision::RevB
                                  : HardwareRevision::RevA;

    if (!m_storage.readBlock(kOffsetSerialNumber,
                              reinterpret_cast<std::uint8_t *>(loadedConfig.serialNumber.data()),
                              kSerialNumberStoredLen))
    {
        return false;
    }
    loadedConfig.serialNumber[kSerialNumberStoredLen] = '\0';

    outConfig = loadedConfig;
    return true;
}
