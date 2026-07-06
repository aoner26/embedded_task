/**
 * @file    II2cBus.hpp
 * @brief   Abstraction for a generic I2C master bus.
 *
 * Device drivers (Eeprom24LC, and anything else that might eventually
 * share the bus) depend on this interface rather than on McuI2cBus
 * directly, which is what lets the EEPROM driver be unit tested against
 * a fake bus instead of real registers.
 */

#ifndef DRIVER_II2CBUS_HPP
#define DRIVER_II2CBUS_HPP

#include <cstddef>
#include <cstdint>

class II2cBus
{
public:
    virtual ~II2cBus() = default;

    /// Single START..STOP write transaction.
    virtual bool write(std::uint8_t slaveAddr7b, const std::uint8_t *txBuf, std::size_t txLen) = 0;

    /// Write transaction immediately followed by a repeated-START read -
    /// the classic "set the register pointer, then read the data back" pattern.
    virtual bool writeThenRead(std::uint8_t slaveAddr7b,
                                const std::uint8_t *txBuf, std::size_t txLen,
                                std::uint8_t *rxBuf, std::size_t rxLen) = 0;
};

#endif // DRIVER_II2CBUS_HPP
