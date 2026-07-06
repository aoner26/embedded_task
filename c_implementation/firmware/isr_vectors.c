/**
 * @file    isr_vectors.c
 * @brief   Interrupt service routine bodies referenced from the vector
 *          table in startup/startup_gcc.c.
 *
 * Kept deliberately thin: the handler only clears the hardware flag and
 * hands off to the application layer's tick function, which is where
 * the actual sampling/hysteresis/LED logic lives. That split is what
 * lets app_temp_monitor_on_sample_tick() be called directly by the PC
 * demonstration program (or a unit test) without needing a real timer
 * to fire it.
 *
 * On the PC build nothing ever triggers this handler - there is no
 * hardware timer counting down - but per the task requirements it is
 * still fully implemented here rather than left as a stub.
 */

#include "app/app_temp_monitor.h"
#include "driver/timer_driver.h"

void TIM2_IRQHandler(void)
{
    tmr_drv_clear_interrupt_flag();
    app_temp_monitor_on_sample_tick();
}
