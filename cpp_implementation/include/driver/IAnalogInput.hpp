/**
 * @file    IAnalogInput.hpp
 * @brief   Abstraction for a single-channel ADC input.
 */

#ifndef DRIVER_IANALOGINPUT_HPP
#define DRIVER_IANALOGINPUT_HPP

#include <cstdint>

class IAnalogInput
{
public:
    virtual ~IAnalogInput() = default;

    /// Performs one blocking conversion and returns the raw, unscaled code.
    virtual std::uint16_t readRaw() = 0;
};

#endif // DRIVER_IANALOGINPUT_HPP
