#include "driver/i2c_driver.h"
#include "platform/mcu_registers.h"
#include "platform/platform_hooks.h"

static void i2c_wait_flag_set(volatile uint32_t *status_reg, uint32_t flag_mask)
{
    platform_hw_service();
    while ((*status_reg & flag_mask) == 0u)
    {
        platform_hw_service();
    }
}

static void i2c_start_condition(void)
{
    I2C1->CR1 |= I2C_CR1_START;
    i2c_wait_flag_set(&I2C1->SR1, I2C_SR1_SB);
}

static void i2c_stop_condition(void)
{
    I2C1->CR1 |= I2C_CR1_STOP;
    platform_hw_service();
}

/* Sends the slave address together with the R/W bit and waits for the
 * slave to acknowledge it. Returns false if nothing answered on the bus
 * (e.g. wrong address, EEPROM not present) so the caller can bail out
 * instead of hanging on a byte that will never arrive. */
static bool i2c_address_slave(uint8_t slave_addr_7b, bool read_transfer)
{
    uint8_t addr_byte = (uint8_t)(((uint32_t)slave_addr_7b << 1) | (read_transfer ? 1u : 0u));

    I2C1->DR = addr_byte;
    platform_hw_service();

    if ((I2C1->SR1 & I2C_SR1_ADDR) == 0u)
    {
        return false; /* NACK - no such device on the bus */
    }

    /* Reading SR1 followed by SR2 is what actually clears ADDR on real
     * silicon (the two-register-read sequence is documented behaviour,
     * not an accident) - reproduced here for the same reason. */
    (void)I2C1->SR1;
    (void)I2C1->SR2;
    I2C1->SR1 &= ~I2C_SR1_ADDR;

    return true;
}

static void i2c_write_one_byte(uint8_t byte_value)
{
    I2C1->DR = byte_value;
    i2c_wait_flag_set(&I2C1->SR1, I2C_SR1_TXE);
    I2C1->SR1 &= ~I2C_SR1_TXE;
}

static uint8_t i2c_read_one_byte(void)
{
    i2c_wait_flag_set(&I2C1->SR1, I2C_SR1_RXNE);
    uint8_t byte_value = (uint8_t)I2C1->DR;
    I2C1->SR1 &= ~I2C_SR1_RXNE;
    return byte_value;
}

void i2c_drv_init(void)
{
    I2C1->CR1 |= I2C_CR1_PE;
}

bool i2c_drv_write(uint8_t slave_addr_7b, const uint8_t *tx_buf, size_t tx_len)
{
    i2c_start_condition();

    if (!i2c_address_slave(slave_addr_7b, false))
    {
        i2c_stop_condition();
        return false;
    }

    for (size_t i = 0; i < tx_len; ++i)
    {
        i2c_write_one_byte(tx_buf[i]);
    }

    i2c_stop_condition();
    return true;
}

bool i2c_drv_write_then_read(uint8_t slave_addr_7b,
                              const uint8_t *tx_buf, size_t tx_len,
                              uint8_t *rx_buf, size_t rx_len)
{
    i2c_start_condition();

    if (!i2c_address_slave(slave_addr_7b, false))
    {
        i2c_stop_condition();
        return false;
    }

    for (size_t i = 0; i < tx_len; ++i)
    {
        i2c_write_one_byte(tx_buf[i]);
    }

    /* Repeated start: no STOP is issued between the write and the read,
     * which is what keeps the EEPROM's internal address pointer from
     * being released back to the last-latched value on some parts. */
    i2c_start_condition();

    if (!i2c_address_slave(slave_addr_7b, true))
    {
        i2c_stop_condition();
        return false;
    }

    for (size_t i = 0; i < rx_len; ++i)
    {
        rx_buf[i] = i2c_read_one_byte();
    }

    i2c_stop_condition();
    return true;
}
