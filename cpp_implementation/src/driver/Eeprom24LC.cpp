#include "driver/Eeprom24LC.hpp"

Eeprom24LC::Eeprom24LC(II2cBus &bus, std::uint8_t slaveAddress)
    : m_bus(bus)
    , m_slaveAddress(slaveAddress)
{
}

bool Eeprom24LC::readBlock(std::uint8_t offset, std::uint8_t *buffer, std::size_t length)
{
    return m_bus.writeThenRead(m_slaveAddress, &offset, 1u, buffer, length);
}
