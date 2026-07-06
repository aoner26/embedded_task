/**
 * @file    mock_registers.cpp
 * @brief   RAM-backed stand-in for the peripheral registers on a PC build.
 *
 * See the header comment in platform/mcu_registers.hpp - this is the
 * only file that knows GPIOA/ADC1/I2C1/TIM2 are not real addresses.
 */

#include "platform/mcu_registers.hpp"

namespace mcu
{
namespace
{
GpioRegs g_gpioaInstance{};
AdcRegs  g_adc1Instance{};
I2cRegs  g_i2c1Instance{};
TimRegs  g_tim2Instance{};
} // namespace

GpioRegs *const GPIOA = &g_gpioaInstance;
AdcRegs  *const ADC1  = &g_adc1Instance;
I2cRegs  *const I2C1  = &g_i2c1Instance;
TimRegs  *const TIM2  = &g_tim2Instance;

} // namespace mcu
