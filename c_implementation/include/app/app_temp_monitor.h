/**
 * @file    app_temp_monitor.h
 * @brief   Application layer - wires the drivers and services together
 *          and owns the startup sequencing.
 */

#ifndef APP_APP_TEMP_MONITOR_H
#define APP_APP_TEMP_MONITOR_H

#include <stdbool.h>

#include "service/device_config_service.h"

/**
 * @brief One-time system bring-up.
 *
 * Brings up every driver, performs the single blocking EEPROM read that
 * is needed to know which sensor is fitted, and arms the sampling timer
 * (without starting it yet). Must complete before
 * app_temp_monitor_start() is called - see the .c file for why the
 * EEPROM access in particular cannot be deferred into the real-time
 * task.
 *
 * @return true if start-up succeeded, false if the EEPROM could not be
 *         read (board configuration is unknown, so it is not safe to
 *         start sampling).
 */
bool app_temp_monitor_init(void);

/**
 * @brief Starts the periodic sampling timer.
 *
 * From this call onward, app_temp_monitor_on_sample_tick() runs every
 * 100 us via TIM2's update interrupt.
 */
void app_temp_monitor_start(void);

/**
 * @brief Body of the real-time sampling task.
 *
 * Reads one raw ADC sample, updates the temperature zone (with
 * hysteresis) and drives the LEDs accordingly. Kept as a plain,
 * directly callable function - rather than being written inline inside
 * the ISR - so the exact same logic can be driven from a test harness
 * or, as here, from the PC demonstration program.
 */
void app_temp_monitor_on_sample_tick(void);

/**
 * @brief Returns the board configuration read out during app_temp_monitor_init().
 *
 * For display/diagnostic purposes only (e.g. a boot splash screen) -
 * nothing on the sampling path depends on this being called.
 */
const device_config_t *app_temp_monitor_get_board_config(void);

#endif /* APP_APP_TEMP_MONITOR_H */
