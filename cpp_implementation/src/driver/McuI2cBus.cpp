#include "driver/McuI2cBus.hpp"
#include "platform/mcu_registers.hpp"
#include "platform/platform_hooks.hpp"

namespace
{
void waitForFlag(volatile std::uint32_t &statusReg, std::uint32_t flagMask)
{
    mcu::hwService();
    while ((statusReg & flagMask) == 0u)
    {
        mcu::hwService();
    }
}
} // namespace

void McuI2cBus::enable()
{
    mcu::I2C1->cr1 |= mcu::kI2cCr1Pe;
}

void McuI2cBus::generateStart()
{
    mcu::I2C1->cr1 |= mcu::kI2cCr1Start;
    waitForFlag(mcu::I2C1->sr1, mcu::kI2cSr1Sb);
}

void McuI2cBus::generateStop()
{
    mcu::I2C1->cr1 |= mcu::kI2cCr1Stop;
    mcu::hwService();
}

bool McuI2cBus::addressSlave(std::uint8_t slaveAddr7b, bool readTransfer)
{
    const std::uint8_t addressByte =
        static_cast<std::uint8_t>((static_cast<std::uint32_t>(slaveAddr7b) << 1) | (readTransfer ? 1u : 0u));

    mcu::I2C1->dr = addressByte;
    mcu::hwService();

    if ((mcu::I2C1->sr1 & mcu::kI2cSr1Addr) == 0u)
    {
        return false; // NACK - no such device on the bus
    }

    // Reading SR1 followed by SR2 is what actually clears ADDR on real
    // silicon - reproduced here for the same reason, not by accident.
    (void)mcu::I2C1->sr1;
    (void)mcu::I2C1->sr2;
    mcu::I2C1->sr1 &= ~mcu::kI2cSr1Addr;

    return true;
}

void McuI2cBus::writeOneByte(std::uint8_t byteValue)
{
    mcu::I2C1->dr = byteValue;
    waitForFlag(mcu::I2C1->sr1, mcu::kI2cSr1Txe);
    mcu::I2C1->sr1 &= ~mcu::kI2cSr1Txe;
}

std::uint8_t McuI2cBus::readOneByte()
{
    waitForFlag(mcu::I2C1->sr1, mcu::kI2cSr1Rxne);
    const std::uint8_t byteValue = static_cast<std::uint8_t>(mcu::I2C1->dr);
    mcu::I2C1->sr1 &= ~mcu::kI2cSr1Rxne;
    return byteValue;
}

bool McuI2cBus::write(std::uint8_t slaveAddr7b, const std::uint8_t *txBuf, std::size_t txLen)
{
    generateStart();

    if (!addressSlave(slaveAddr7b, false))
    {
        generateStop();
        return false;
    }

    for (std::size_t i = 0; i < txLen; ++i)
    {
        writeOneByte(txBuf[i]);
    }

    generateStop();
    return true;
}

bool McuI2cBus::writeThenRead(std::uint8_t slaveAddr7b,
                               const std::uint8_t *txBuf, std::size_t txLen,
                               std::uint8_t *rxBuf, std::size_t rxLen)
{
    generateStart();

    if (!addressSlave(slaveAddr7b, false))
    {
        generateStop();
        return false;
    }

    for (std::size_t i = 0; i < txLen; ++i)
    {
        writeOneByte(txBuf[i]);
    }

    // Repeated start: no STOP between the write and the read, which is
    // what keeps the EEPROM's internal address pointer latched.
    generateStart();

    if (!addressSlave(slaveAddr7b, true))
    {
        generateStop();
        return false;
    }

    for (std::size_t i = 0; i < rxLen; ++i)
    {
        rxBuf[i] = readOneByte();
    }

    generateStop();
    return true;
}
