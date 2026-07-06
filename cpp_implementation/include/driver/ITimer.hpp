/**
 * @file    ITimer.hpp
 * @brief   Abstraction for the periodic hardware timer driving the
 *          real-time sampling task.
 */

#ifndef DRIVER_ITIMER_HPP
#define DRIVER_ITIMER_HPP

#include <cstdint>

class ITimer
{
public:
    virtual ~ITimer() = default;

    /// Arms the timer for a periodic interrupt every periodUs microseconds.
    /// Does not start the counter - call start() once the rest of the
    /// system is ready to receive ticks.
    virtual void configurePeriodicMicros(std::uint32_t timerClockHz, std::uint32_t periodUs) = 0;

    /// Starts the free-running counter and enables its interrupt.
    virtual void start() = 0;

    /// Clears the pending update-interrupt flag - must be called from
    /// within the ISR before it returns.
    virtual void clearInterruptFlag() = 0;
};

#endif // DRIVER_ITIMER_HPP
