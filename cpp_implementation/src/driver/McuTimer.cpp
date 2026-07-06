#include "driver/McuTimer.hpp"
#include "platform/mcu_registers.hpp"

void McuTimer::configurePeriodicMicros(std::uint32_t timerClockHz, std::uint32_t periodUs)
{
    // Run the prescaler down to a clean 1 MHz tick (1 count = 1 us) so the
    // auto-reload value can be computed directly from the requested period
    // without dragging fractional microseconds into the jitter budget.
    constexpr std::uint32_t kTimerTickHz = 1000000u;

    mcu::TIM2->psc = (timerClockHz / kTimerTickHz) - 1u;
    mcu::TIM2->arr = periodUs - 1u;
    mcu::TIM2->cnt = 0u;
    mcu::TIM2->dier |= mcu::kTimDierUie;
}

void McuTimer::start()
{
    mcu::TIM2->cr1 |= mcu::kTimCr1Cen;
}

void McuTimer::clearInterruptFlag()
{
    mcu::TIM2->sr &= ~mcu::kTimSrUif;
}
