/**
 * @file    McuTimer.hpp
 * @brief   Concrete general-purpose-timer driver that generates the
 *          periodic sampling interrupt for the real-time task.
 */

#ifndef DRIVER_MCUTIMER_HPP
#define DRIVER_MCUTIMER_HPP

#include "driver/ITimer.hpp"

class McuTimer final : public ITimer
{
public:
    void configurePeriodicMicros(std::uint32_t timerClockHz, std::uint32_t periodUs) override;
    void start() override;
    void clearInterruptFlag() override;
};

#endif // DRIVER_MCUTIMER_HPP
