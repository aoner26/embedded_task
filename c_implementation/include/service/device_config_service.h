/**
 * @file    device_config_service.h
 * @brief   One-time board configuration read-out.
 *
 * The hardware revision and serial number never change while the board
 * is running - they are read from the EEPROM exactly once, at boot,
 * before the real-time sampling timer is ever armed. See app_temp_monitor.c
 * for the reasoning: an I2C transaction is far too slow and too jittery
 * to ever be allowed inside the 100 us periodic task.
 */

#ifndef SERVICE_DEVICE_CONFIG_SERVICE_H
#define SERVICE_DEVICE_CONFIG_SERVICE_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    HW_REVISION_A = 0,  /* 1.0 degC per ADC count   */
    HW_REVISION_B = 1   /* 0.1 degC per ADC count   */
} hw_revision_t;

#define DEVICE_SERIAL_NUMBER_MAX_LEN  8u   /* 7 printable chars + '\0' */

typedef struct
{
    hw_revision_t hw_revision;
    char          serial_number[DEVICE_SERIAL_NUMBER_MAX_LEN];
} device_config_t;

/**
 * @brief Blocking read of the board configuration from the EEPROM.
 *
 * @param out_config Destination for the configuration read out.
 * @return true if the EEPROM answered and the config was read, false
 *         otherwise (out_config is left untouched on failure).
 */
bool cfg_svc_load_from_eeprom(device_config_t *out_config);

#endif /* SERVICE_DEVICE_CONFIG_SERVICE_H */
