#include "driver/timer_driver.h"
#include "platform/mcu_registers.h"

void tmr_drv_configure_periodic_us(uint32_t timer_clock_hz, uint32_t period_us)
{
    /* Run the prescaler down to a clean 1 MHz tick (1 count = 1 us) so
     * the auto-reload value can be computed directly from the requested
     * period without dragging fractional microseconds into the jitter
     * budget. */
    const uint32_t timer_tick_hz = 1000000u;

    TIM2->PSC = (timer_clock_hz / timer_tick_hz) - 1u;
    TIM2->ARR = period_us - 1u;
    TIM2->CNT = 0u;
    TIM2->DIER |= TIM_DIER_UIE;
}

void tmr_drv_start(void)
{
    TIM2->CR1 |= TIM_CR1_CEN;
}

void tmr_drv_clear_interrupt_flag(void)
{
    TIM2->SR &= ~TIM_SR_UIF;
}
