/**
 * @file    platform_hooks.h
 * @brief   Seam between a driver and whatever is actually sitting on the bus.
 */

#ifndef PLATFORM_PLATFORM_HOOKS_H
#define PLATFORM_PLATFORM_HOOKS_H

/**
 * @brief Gives the I2C peripheral a chance to make forward progress.
 *
 * On real silicon the I2C shift register moves bits in and out on its own
 * clock domain while the CPU just polls SR1/SR2 - there is nothing for
 * software to actively do, so a target build would define this as an
 * empty inline. This demonstration has no physical bus behind ADC1/I2C1,
 * so platform_sim/platform_hooks_sim.c uses this hook to advance the
 * simulated bus/EEPROM model instead. i2c_driver.c calls it in every
 * wait loop and does not know (or care) which of the two it is linked
 * against.
 */
void platform_hw_service(void);

#endif /* PLATFORM_PLATFORM_HOOKS_H */
