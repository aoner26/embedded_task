/**
 * @file    McuI2cBus.hpp
 * @brief   Concrete polling I2C master bus driver.
 */

#ifndef DRIVER_MCUI2CBUS_HPP
#define DRIVER_MCUI2CBUS_HPP

#include <cstddef>
#include <cstdint>

#include "driver/II2cBus.hpp"

class McuI2cBus final : public II2cBus
{
public:
    /// Enables the I2C peripheral in master mode.
    void enable();

    bool write(std::uint8_t slaveAddr7b, const std::uint8_t *txBuf, std::size_t txLen) override;

    bool writeThenRead(std::uint8_t slaveAddr7b,
                        const std::uint8_t *txBuf, std::size_t txLen,
                        std::uint8_t *rxBuf, std::size_t rxLen) override;

private:
    static void generateStart();
    static void generateStop();
    static bool addressSlave(std::uint8_t slaveAddr7b, bool readTransfer);
    static void writeOneByte(std::uint8_t byteValue);
    static std::uint8_t readOneByte();
};

#endif // DRIVER_MCUI2CBUS_HPP
