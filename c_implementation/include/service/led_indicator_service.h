/**
 * @file    led_indicator_service.h
 * @brief   Maps a temperature zone onto the three status LEDs.
 */

#ifndef SERVICE_LED_INDICATOR_SERVICE_H
#define SERVICE_LED_INDICATOR_SERVICE_H

#include "service/temperature_service.h"

/**
 * @brief Configures the three LED pins as outputs and switches them off.
 */
void led_svc_init(void);

/**
 * @brief Lights the single LED that corresponds to the given zone,
 *        making sure the other two are off.
 */
void led_svc_update(temp_zone_t zone);

#endif /* SERVICE_LED_INDICATOR_SERVICE_H */
