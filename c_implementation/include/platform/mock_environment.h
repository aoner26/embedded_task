/**
 * @file    mock_environment.h
 * @brief   Test-bench style hooks used only by the PC demo harness.
 *
 * These are not part of the driver/service/application stack - they let
 * the demo program stand in for "the physical world" (a pre-programmed
 * EEPROM chip and a temperature sensor voltage sweeping across the
 * ranges of interest) so the real firmware layers can be exercised
 * without any actual hardware attached.
 */

#ifndef PLATFORM_MOCK_ENVIRONMENT_H
#define PLATFORM_MOCK_ENVIRONMENT_H

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Pre-programs the simulated EEPROM, as if it came from the factory.
 */
void mock_eeprom_program(uint16_t offset, const uint8_t *data, size_t len);

/**
 * @brief Places a new raw sample in the mock ADC's data register.
 *
 * Stands in for the sensor driving a voltage onto the ADC input pin -
 * on real hardware adc_drv_read_raw() would trigger an actual
 * conversion of whatever is on the pin at that moment.
 */
void mock_adc_set_raw_sample(uint16_t raw_value);

#endif /* PLATFORM_MOCK_ENVIRONMENT_H */
