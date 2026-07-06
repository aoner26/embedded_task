/**
 * @file    McuAdcChannel.hpp
 * @brief   Concrete single-channel ADC driver, used for the temperature sensor.
 *
 * The conversion is triggered and polled to completion here - it
 * finishes fast enough to stay well inside the 100 us sampling budget.
 * A slower sensor would need this to become interrupt/DMA driven
 * instead of blocking on EOC like this one does.
 */

#ifndef DRIVER_MCUADCCHANNEL_HPP
#define DRIVER_MCUADCCHANNEL_HPP

#include <cstdint>

#include "driver/IAnalogInput.hpp"

class McuAdcChannel final : public IAnalogInput
{
public:
    explicit McuAdcChannel(std::uint8_t channelNumber);

    /// Selects the input channel and enables the ADC.
    void enable();

    std::uint16_t readRaw() override;

private:
    std::uint8_t m_channelNumber;
};

#endif // DRIVER_MCUADCCHANNEL_HPP
