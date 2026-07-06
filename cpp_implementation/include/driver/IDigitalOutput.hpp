/**
 * @file    IDigitalOutput.hpp
 * @brief   Abstraction for a single GPIO output pin.
 *
 * The service layer drives LEDs through this interface, never through a
 * concrete MCU class - that is what lets LedIndicator be built and
 * tested without any real (or even simulated) register in sight.
 */

#ifndef DRIVER_IDIGITALOUTPUT_HPP
#define DRIVER_IDIGITALOUTPUT_HPP

class IDigitalOutput
{
public:
    virtual ~IDigitalOutput() = default;

    /// Drives the pin high (true) or low (false).
    virtual void set(bool driveHigh) = 0;
};

#endif // DRIVER_IDIGITALOUTPUT_HPP
