/**
 * @file    timer_driver.h
 * @brief   General purpose timer driver used to generate the periodic
 *          sampling interrupt that drives the real-time task.
 */

#ifndef DRIVER_TIMER_DRIVER_H
#define DRIVER_TIMER_DRIVER_H

#include <stdint.h>

/**
 * @brief Configures TIM2 to raise an update interrupt every period_us.
 *
 * @param timer_clock_hz Frequency of the timer's input clock.
 * @param period_us      Desired interrupt period, in microseconds.
 *
 * The counter is not started here - call tmr_drv_start() once the rest
 * of the system (in particular the one-time EEPROM configuration read)
 * has completed, so the very first tick does not fire before everything
 * downstream is ready to handle it.
 */
void tmr_drv_configure_periodic_us(uint32_t timer_clock_hz, uint32_t period_us);

/**
 * @brief Starts the free-running counter and enables its interrupt.
 */
void tmr_drv_start(void);

/**
 * @brief Clears the pending update-interrupt flag.
 *
 * Must be called from within the ISR before returning, otherwise the
 * interrupt controller would immediately re-enter the handler.
 */
void tmr_drv_clear_interrupt_flag(void);

#endif /* DRIVER_TIMER_DRIVER_H */
