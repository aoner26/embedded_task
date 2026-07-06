/**
 * @file    platform_hooks.hpp
 * @brief   Seam between a driver and whatever is actually sitting on the bus.
 */

#ifndef PLATFORM_PLATFORM_HOOKS_HPP
#define PLATFORM_PLATFORM_HOOKS_HPP

namespace mcu
{

/**
 * @brief Gives the simulated peripherals a chance to make forward progress.
 *
 * On real silicon the ADC/I2C peripherals run on their own clock domain
 * while the CPU just polls status bits - there is nothing for software
 * to actively do, so a target build would define this as an empty
 * inline. This demonstration has no physical bus behind ADC1/I2C1, so
 * platform_sim/platform_hooks_sim.cpp uses this hook to advance the
 * simulated ADC and EEPROM models instead. McuAdcChannel and McuI2cBus
 * call it from every wait loop and do not know (or care) which of the
 * two implementations they end up linked against.
 */
void hwService();

} // namespace mcu

#endif // PLATFORM_PLATFORM_HOOKS_HPP
