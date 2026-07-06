/**
 * @file    INonVolatileStorage.hpp
 * @brief   Abstraction for "some place that remembers configuration
 *          across a power cycle".
 *
 * DeviceConfigManager depends on this, not on Eeprom24LC or I2C - as
 * far as the service layer is concerned, the configuration could just
 * as well be coming from SPI flash. This is dependency inversion in
 * practice: the high-level policy (what the configuration means)
 * neither knows nor cares how the low-level detail (how it is fetched)
 * is implemented.
 */

#ifndef DRIVER_INONVOLATILESTORAGE_HPP
#define DRIVER_INONVOLATILESTORAGE_HPP

#include <cstddef>
#include <cstdint>

class INonVolatileStorage
{
public:
    virtual ~INonVolatileStorage() = default;

    /// Reads length bytes starting at offset into buffer.
    /// @return true on success, false if the backing device did not respond.
    virtual bool readBlock(std::uint8_t offset, std::uint8_t *buffer, std::size_t length) = 0;
};

#endif // DRIVER_INONVOLATILESTORAGE_HPP
