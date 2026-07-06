/**
 * @file    DeviceConfig.hpp
 * @brief   Board configuration as read out of the EEPROM.
 */

#ifndef SERVICE_DEVICECONFIG_HPP
#define SERVICE_DEVICECONFIG_HPP

#include <array>
#include <cstdint>

enum class HardwareRevision : std::uint8_t
{
    RevA = 0, ///< 1.0 degC per ADC count
    RevB = 1  ///< 0.1 degC per ADC count
};

constexpr std::size_t kSerialNumberMaxLen = 8; ///< 7 printable chars + '\0'

struct DeviceConfig
{
    HardwareRevision              hwRevision   = HardwareRevision::RevA;
    std::array<char, kSerialNumberMaxLen> serialNumber{};
};

#endif // SERVICE_DEVICECONFIG_HPP
