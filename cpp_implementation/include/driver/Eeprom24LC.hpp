/**
 * @file    Eeprom24LC.hpp
 * @brief   Device driver for a 24LCxx-style byte-addressable I2C EEPROM.
 *
 * Depends on II2cBus (constructor injection) rather than owning or even
 * knowing about McuI2cBus directly - the bus implementation could be
 * swapped for a bit-banged one, or a mock for unit tests, with this
 * class never noticing.
 */

#ifndef DRIVER_EEPROM24LC_HPP
#define DRIVER_EEPROM24LC_HPP

#include <cstdint>

#include "driver/II2cBus.hpp"
#include "driver/INonVolatileStorage.hpp"

class Eeprom24LC final : public INonVolatileStorage
{
public:
    explicit Eeprom24LC(II2cBus &bus, std::uint8_t slaveAddress = 0x50);

    bool readBlock(std::uint8_t offset, std::uint8_t *buffer, std::size_t length) override;

private:
    II2cBus     &m_bus;
    std::uint8_t m_slaveAddress;
};

#endif // DRIVER_EEPROM24LC_HPP
