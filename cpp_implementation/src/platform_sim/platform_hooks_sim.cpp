/**
 * @file    platform_hooks_sim.cpp
 * @brief   PC-only stand-in for "the rest of the I2C bus" (and the ADC).
 *
 * McuI2cBus / McuAdcChannel are written exactly the way they would be
 * for real silicon: set a control bit, then poll a status bit until the
 * hardware reports progress. On the bench that progress is made by the
 * peripheral's own state machine; here there is no peripheral, so this
 * file plays that role instead. For I2C it behaves like a single EEPROM
 * slave, including the "internal address pointer persists across a
 * repeated START" behaviour real byte-addressable EEPROMs have - which
 * is exactly what lets Eeprom24LC do a "write pointer, then read"
 * transaction.
 */

#include "platform/mcu_registers.hpp"
#include "platform/mock_environment.hpp"
#include "platform/platform_hooks.hpp"

#include <cstring>

namespace mcu
{
namespace
{

constexpr std::size_t  kMockEepromSizeBytes    = 64;
constexpr std::uint8_t kMockEepromSlaveAddress = 0x50;

enum class BusPhase
{
    Idle,
    AwaitingAddressByte,
    Transferring
};

std::uint8_t s_eepromMemory[kMockEepromSizeBytes]{};
std::uint8_t s_eepromPointer        = 0;
bool         s_pointerIsLatched     = false;
BusPhase     s_phase                = BusPhase::Idle;
bool         s_currentTransferIsRead = false;

void handleStartCondition()
{
    I2C1->cr1 &= ~kI2cCr1Start;
    I2C1->sr1 |= kI2cSr1Sb;
    I2C1->sr2 |= kI2cSr2Busy;
    s_phase = BusPhase::AwaitingAddressByte;
}

void handleStopCondition()
{
    I2C1->cr1 &= ~kI2cCr1Stop;
    I2C1->sr2 &= ~kI2cSr2Busy;
    s_phase = BusPhase::Idle;
    s_pointerIsLatched = false;
}

void handleAddressByte(std::uint8_t addressByte)
{
    const std::uint8_t deviceAddr    = static_cast<std::uint8_t>(addressByte >> 1);
    const bool         readRequested = (addressByte & 0x01u) != 0u;

    if (deviceAddr != kMockEepromSlaveAddress)
    {
        return; // no device answers -> ADDR flag stays clear -> caller sees a NACK
    }

    s_currentTransferIsRead = readRequested;
    I2C1->sr1 |= kI2cSr1Addr;
    s_phase = BusPhase::Transferring;

    if (readRequested)
    {
        // First byte of the read is already sitting in DR by the time the
        // caller comes looking for RXNE, exactly like a real EEPROM that
        // starts shifting out data right after the address is ACKed.
        I2C1->dr = s_eepromMemory[s_eepromPointer];
        I2C1->sr1 |= kI2cSr1Rxne;
    }
}

void handleDataByteWritten(std::uint8_t byteValue)
{
    if (!s_pointerIsLatched)
    {
        // First byte after the address phase of a write is the memory
        // offset to operate on.
        s_eepromPointer    = static_cast<std::uint8_t>(byteValue % kMockEepromSizeBytes);
        s_pointerIsLatched = true;
    }
    else
    {
        s_eepromMemory[s_eepromPointer] = byteValue;
        s_eepromPointer = static_cast<std::uint8_t>((s_eepromPointer + 1u) % kMockEepromSizeBytes);
    }

    I2C1->sr1 |= kI2cSr1Txe;
}

void handleReadService()
{
    // Caller already consumed the byte we put up (RXNE was cleared) -
    // shift the next one out, mimicking a sequential read.
    if ((I2C1->sr1 & kI2cSr1Rxne) == 0u)
    {
        s_eepromPointer = static_cast<std::uint8_t>((s_eepromPointer + 1u) % kMockEepromSizeBytes);
        I2C1->dr = s_eepromMemory[s_eepromPointer];
        I2C1->sr1 |= kI2cSr1Rxne;
    }
}

void serviceAdcPeripheral()
{
    if ((ADC1->cr & kAdcCrStart) != 0u)
    {
        // The mocked sensor has no real settling/sampling time, so the
        // "conversion" is complete as soon as it is requested.
        ADC1->cr &= ~kAdcCrStart;
        ADC1->sr |= kAdcSrEoc;
    }
}

} // namespace

void mockEepromProgram(std::uint16_t offset, const std::uint8_t *data, std::size_t length)
{
    // Represents the EEPROM having been pre-programmed on the production
    // line - by the time firmware boots, this content is just "already
    // there". Used only by the PC demo harness to set the stage.
    std::memcpy(&s_eepromMemory[offset], data, length);
}

void mockAdcSetRawSample(std::uint16_t rawValue)
{
    ADC1->dr = rawValue;
}

void hwService()
{
    serviceAdcPeripheral();

    if ((I2C1->cr1 & kI2cCr1Start) != 0u)
    {
        handleStartCondition();
        return;
    }

    if ((I2C1->cr1 & kI2cCr1Stop) != 0u)
    {
        handleStopCondition();
        return;
    }

    switch (s_phase)
    {
        case BusPhase::AwaitingAddressByte:
            if ((I2C1->sr1 & kI2cSr1Addr) == 0u)
            {
                handleAddressByte(static_cast<std::uint8_t>(I2C1->dr));
            }
            break;

        case BusPhase::Transferring:
            if (s_currentTransferIsRead)
            {
                handleReadService();
            }
            else if ((I2C1->sr1 & kI2cSr1Txe) == 0u)
            {
                handleDataByteWritten(static_cast<std::uint8_t>(I2C1->dr));
            }
            break;

        case BusPhase::Idle:
        default:
            break;
    }
}

} // namespace mcu
