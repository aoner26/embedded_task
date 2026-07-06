/**
 * @file    i2c_driver.h
 * @brief   Polling I2C master driver.
 *
 * This is a generic bus driver - it knows how to talk to a 7-bit
 * addressed I2C slave, but nothing about EEPROMs specifically. The
 * EEPROM device driver (eeprom_driver.c) is built on top of it.
 */

#ifndef DRIVER_I2C_DRIVER_H
#define DRIVER_I2C_DRIVER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Enables the I2C peripheral in master mode.
 */
void i2c_drv_init(void);

/**
 * @brief Writes a buffer to a slave in a single START..STOP transaction.
 */
bool i2c_drv_write(uint8_t slave_addr_7b, const uint8_t *tx_buf, size_t tx_len);

/**
 * @brief Writes a buffer, then repeated-starts into a read of another buffer.
 *
 * This is the classic "set the register/memory pointer, then read back the
 * data" transaction that EEPROMs and most sensor ICs use.
 */
bool i2c_drv_write_then_read(uint8_t slave_addr_7b,
                              const uint8_t *tx_buf, size_t tx_len,
                              uint8_t *rx_buf, size_t rx_len);

#endif /* DRIVER_I2C_DRIVER_H */
