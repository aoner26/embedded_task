/**
 * @file    McuGpioOutput.hpp
 * @brief   Concrete GPIOA output pin driver.
 */

#ifndef DRIVER_MCUGPIOOUTPUT_HPP
#define DRIVER_MCUGPIOOUTPUT_HPP

#include <cstdint>

#include "driver/IDigitalOutput.hpp"

class McuGpioOutput final : public IDigitalOutput
{
public:
    explicit McuGpioOutput(std::uint8_t pinNumber);

    /// Configures the pin as a push-pull output. Must be called once
    /// before the pin is used.
    void configure();

    void set(bool driveHigh) override;

private:
    std::uint8_t m_pinNumber;
};

#endif // DRIVER_MCUGPIOOUTPUT_HPP
