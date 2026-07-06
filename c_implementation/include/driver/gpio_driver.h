/**
 * @file    gpio_driver.h
 * @brief   Minimal GPIO driver - just enough to drive the three status LEDs.
 *
 * This is the lowest layer of the stack (hardware abstraction / driver
 * layer). It only knows about pins and register bits, nothing about what
 * a "temperature status" is - that decision is made further up, in the
 * service layer.
 */

#ifndef DRIVER_GPIO_DRIVER_H
#define DRIVER_GPIO_DRIVER_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Configures the given GPIOA pin as a push-pull digital output.
 * @param pin_no Pin number within port A (0..15).
 */
void gpio_drv_init_output(uint8_t pin_no);

/**
 * @brief Drives a previously configured output pin high or low.
 * @param pin_no    Pin number within port A (0..15).
 * @param drive_high true to drive the pin high, false to drive it low.
 */
void gpio_drv_write_pin(uint8_t pin_no, bool drive_high);

#endif /* DRIVER_GPIO_DRIVER_H */
