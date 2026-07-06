/**
 * @file    mock_registers.c
 * @brief   RAM-backed stand-in for the peripheral registers on a PC build.
 *
 * The driver layer is written against real register structures
 * (see platform/mcu_registers.h) so it looks and behaves exactly like
 * firmware that talks to actual silicon. Since this task only asks for a
 * PC demonstration, we obviously do not have a real memory bus to place
 * those structs on - instead, this file allocates one static instance per
 * peripheral and hands the driver layer a pointer to it, keeping the
 * "GPIOA / ADC1 / I2C1 / TIM2" naming that a real CMSIS device header
 * would provide. Nothing outside this file needs to know the difference.
 */

#include "platform/mcu_registers.h"

static gpio_regs_t s_gpioa_instance;
static adc_regs_t  s_adc1_instance;
static i2c_regs_t  s_i2c1_instance;
static tim_regs_t  s_tim2_instance;

gpio_regs_t *const GPIOA = &s_gpioa_instance;
adc_regs_t  *const ADC1  = &s_adc1_instance;
i2c_regs_t  *const I2C1  = &s_i2c1_instance;
tim_regs_t  *const TIM2  = &s_tim2_instance;
