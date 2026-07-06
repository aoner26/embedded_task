/**
 * @file    eeprom_driver.h
 * @brief   Device driver for the on-board configuration EEPROM.
 *
 * Built on top of i2c_driver.h - this file only adds the knowledge of
 * "there's a byte-addressable EEPROM at this bus address", not what the
 * bytes actually mean (that is the config service's job, one layer up).
 */

#ifndef DRIVER_EEPROM_DRIVER_H
#define DRIVER_EEPROM_DRIVER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define EEPROM_DRV_SLAVE_ADDRESS   0x50u

void eeprom_drv_init(void);

/**
 * @brief Reads len bytes starting at mem_offset into data.
 * @return true on success, false if the device did not respond.
 */
bool eeprom_drv_read(uint8_t mem_offset, uint8_t *data, size_t len);

#endif /* DRIVER_EEPROM_DRIVER_H */
