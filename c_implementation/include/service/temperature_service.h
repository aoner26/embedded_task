/**
 * @file    temperature_service.h
 * @brief   Converts raw ADC samples into a temperature zone.
 *
 * This is where the sensor-revision-dependent scale factor and the
 * hysteresis (anti-flicker) logic live. Everything here has to run in
 * well under 100 us and must never touch a bus, so it only ever works
 * with numbers that are already sitting in RAM.
 */

#ifndef SERVICE_TEMPERATURE_SERVICE_H
#define SERVICE_TEMPERATURE_SERVICE_H

#include <stdint.h>

#include "service/device_config_service.h"

typedef enum
{
    TEMP_ZONE_CRITICAL_LOW = 0,  /* < 5 degC              */
    TEMP_ZONE_NORMAL,            /* 5 .. 85 degC          */
    TEMP_ZONE_WARNING,           /* 85 .. 105 degC        */
    TEMP_ZONE_CRITICAL_HIGH      /* >= 105 degC           */
} temp_zone_t;

/**
 * @brief One-time setup - fixes the ADC-count-to-temperature scale
 *        factor according to which sensor revision is fitted.
 *
 * Must be called after the hardware revision has been read from the
 * EEPROM and before the first call to temp_svc_process_sample().
 */
void temp_svc_init(hw_revision_t hw_revision);

/**
 * @brief Consumes one fresh raw ADC sample.
 *
 * Intended to be called from the 100 us periodic real-time task
 * (directly, or from the timer ISR it is attached to).
 */
void temp_svc_process_sample(uint16_t raw_adc_sample);

/**
 * @brief Latest classified temperature zone (survives until the next
 *        sample moves it past a hysteresis boundary).
 */
temp_zone_t temp_svc_get_zone(void);

/**
 * @brief Latest temperature reading, in tenths of a degree Celsius.
 *
 * Fixed-point on purpose - keeps the whole sampling path free of
 * floating point, which not every target in the field has an FPU for.
 */
int16_t temp_svc_get_temperature_deci_celsius(void);

#endif /* SERVICE_TEMPERATURE_SERVICE_H */
