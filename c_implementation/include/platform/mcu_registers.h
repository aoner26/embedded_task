/**
 * @file    mcu_registers.h
 * @brief   Memory-mapped peripheral register layout for the target MCU.
 *
 * This header describes the register blocks for the peripherals that the
 * driver layer needs (GPIO, ADC, I2C and a general purpose timer). The
 * layout follows the usual "struct of volatile registers overlaid on a
 * fixed address" trick that CMSIS device headers use, so the driver code
 * below reads exactly like it would on real silicon.
 *
 * For the PC demonstration build (TARGET_PC defined by the build system)
 * there is obviously no physical bus to map these structs onto, so
 * platform_sim/mock_registers.c backs every peripheral pointer with a
 * plain RAM instance instead of a real address. Everything above the
 * driver layer is completely unaware of this - it only ever sees the
 * peripheral pointers below (GPIOA, ADC1, I2C1, TIM2).
 *
 * Register bit widths and offsets are simplified on purpose (this is not
 * a datasheet-accurate reproduction of any particular vendor part), but
 * the structure and usage pattern is representative of a real Cortex-M
 * class MCU.
 */

#ifndef PLATFORM_MCU_REGISTERS_H
#define PLATFORM_MCU_REGISTERS_H

#include <stdint.h>

/* --------------------------------------------------------------------- */
/*  GPIO                                                                  */
/* --------------------------------------------------------------------- */

typedef struct
{
    volatile uint32_t MODER;   /* Port mode register (00 = input, 01 = output) */
    volatile uint32_t ODR;     /* Output data register                        */
    volatile uint32_t IDR;     /* Input data register (read-only)             */
} gpio_regs_t;

/* --------------------------------------------------------------------- */
/*  ADC                                                                   */
/* --------------------------------------------------------------------- */

typedef struct
{
    volatile uint32_t CR;      /* Control register (enable, start conversion) */
    volatile uint32_t SR;      /* Status register (EOC = end of conversion)   */
    volatile uint32_t CHSEL;   /* Selected input channel                      */
    volatile uint32_t DR;      /* Data register - last converted raw sample   */
} adc_regs_t;

#define ADC_CR_ADEN         (1u << 0)  /* ADC enable                */
#define ADC_CR_START        (1u << 1)  /* Start a single conversion */
#define ADC_SR_EOC          (1u << 0)  /* End-of-conversion flag    */

/* --------------------------------------------------------------------- */
/*  I2C (master mode only, which is all the EEPROM driver needs)         */
/* --------------------------------------------------------------------- */

typedef struct
{
    volatile uint32_t CR1;     /* Control register (PE, START, STOP, ACK) */
    volatile uint32_t SR1;     /* Status register 1 (SB, ADDR, TxE, RxNE, BTF) */
    volatile uint32_t SR2;     /* Status register 2 (BUSY)                */
    volatile uint32_t DR;      /* Data register                           */
} i2c_regs_t;

#define I2C_CR1_PE          (1u << 0)  /* Peripheral enable */
#define I2C_CR1_START       (1u << 1)  /* Generate (repeated) start condition */
#define I2C_CR1_STOP        (1u << 2)  /* Generate stop condition */

#define I2C_SR1_SB          (1u << 0)  /* Start bit sent */
#define I2C_SR1_ADDR        (1u << 1)  /* Address sent / matched, ACK received */
#define I2C_SR1_TXE         (1u << 2)  /* Data register empty, ready for next TX byte */
#define I2C_SR1_RXNE        (1u << 3)  /* Data register not empty, byte received */
#define I2C_SR1_BTF         (1u << 4)  /* Byte transfer finished */

#define I2C_SR2_BUSY        (1u << 0)  /* Bus busy */

/* --------------------------------------------------------------------- */
/*  General purpose timer, drives the 100 us sampling interrupt          */
/* --------------------------------------------------------------------- */

typedef struct
{
    volatile uint32_t CR1;     /* Control register (counter enable)    */
    volatile uint32_t PSC;     /* Prescaler                            */
    volatile uint32_t ARR;     /* Auto-reload value (defines period)   */
    volatile uint32_t CNT;     /* Free running counter                 */
    volatile uint32_t SR;      /* Status register (UIF = update flag)  */
    volatile uint32_t DIER;    /* DMA/interrupt enable register        */
} tim_regs_t;

#define TIM_CR1_CEN         (1u << 0)  /* Counter enable        */
#define TIM_SR_UIF          (1u << 0)  /* Update interrupt flag */
#define TIM_DIER_UIE        (1u << 0)  /* Update interrupt enable */

/* --------------------------------------------------------------------- */
/*  Peripheral instance pointers                                         */
/*                                                                        */
/*  On real hardware these would be literal addresses taken from the     */
/*  reference manual, e.g. "#define GPIOA ((gpio_regs_t *)0x40010800u)". */
/*  platform_sim/mock_registers.c provides the RAM-backed storage for    */
/*  the PC build and exposes it through the very same symbols so the     */
/*  driver layer never needs an #ifdef.                                  */
/* --------------------------------------------------------------------- */

extern gpio_regs_t *const GPIOA;
extern adc_regs_t  *const ADC1;
extern i2c_regs_t  *const I2C1;
extern tim_regs_t  *const TIM2;

#endif /* PLATFORM_MCU_REGISTERS_H */
