/**
 * @file    adc_driver.h
 * @brief   Single-channel ADC driver used to sample the temperature sensor.
 *
 * The conversion itself is triggered and polled to completion here -
 * the actual sensor happens to convert fast enough that this stays well
 * inside the 100 us sampling budget the real-time task has to honour.
 * If a slower part were ever used, this would need to move to a
 * DMA/interrupt driven scheme instead of the polling one below.
 */

#ifndef DRIVER_ADC_DRIVER_H
#define DRIVER_ADC_DRIVER_H

#include <stdint.h>

/**
 * @brief Enables the ADC and selects the temperature sensor input channel.
 * @param channel_no ADC input channel the sensor is wired to.
 */
void adc_drv_init(uint8_t channel_no);

/**
 * @brief Performs a single blocking conversion and returns the raw sample.
 *
 * The returned value is an unscaled ADC code - converting it into a
 * physical temperature is the responsibility of the temperature service,
 * since the scale factor depends on which sensor revision is fitted.
 */
uint16_t adc_drv_read_raw(void);

#endif /* DRIVER_ADC_DRIVER_H */
