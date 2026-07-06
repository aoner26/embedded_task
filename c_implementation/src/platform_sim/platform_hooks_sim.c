/**
 * @file    platform_hooks_sim.c
 * @brief   PC-only stand-in for "the rest of the I2C bus".
 *
 * i2c_driver.c is written exactly the way it would be for real silicon:
 * it sets a control bit and then polls a status bit until the hardware
 * reports progress. On the bench that progress is made by the I2C
 * peripheral's own state machine; here there is no peripheral, so this
 * file plays that role instead. It watches the same CR1/SR1/DR registers
 * the driver touches and reacts to them as a single EEPROM slave would,
 * including the "internal address pointer" behaviour real EEPROMs have
 * (the address survives a repeated START, which is exactly what lets
 * eeprom_driver.c do a "write pointer, then read" transaction).
 *
 * None of this is visible to, or needed by, code above the driver layer.
 */

#include "platform/mcu_registers.h"
#include "platform/mock_environment.h"
#include "platform/platform_hooks.h"

#include <stdbool.h>
#include <string.h>

#define MOCK_EEPROM_SIZE_BYTES     64u
#define MOCK_EEPROM_SLAVE_ADDRESS  0x50u   /* typical 24LCxx style address */

typedef enum
{
    BUS_PHASE_IDLE = 0,
    BUS_PHASE_AWAITING_ADDR_BYTE,
    BUS_PHASE_TRANSFERRING
} bus_phase_t;

static uint8_t     s_eeprom_memory[MOCK_EEPROM_SIZE_BYTES];
static uint8_t     s_eeprom_pointer;
static bool        s_pointer_is_latched;
static bus_phase_t s_phase = BUS_PHASE_IDLE;
static bool        s_current_transfer_is_read;

void mock_eeprom_program(uint16_t offset, const uint8_t *data, size_t len)
{
    /* Represents the EEPROM having been pre-programmed on the production
     * line - by the time firmware boots, this content is just "already
     * there". Used only by the PC demo harness to set the stage. */
    memcpy(&s_eeprom_memory[offset], data, len);
}

void mock_adc_set_raw_sample(uint16_t raw_value)
{
    /* Stands in for the temperature sensor driving a new voltage onto
     * the ADC input pin - real silicon would sample whatever is present
     * there the next time a conversion is triggered. */
    ADC1->DR = raw_value;
}

static void handle_start_condition(void)
{
    I2C1->CR1 &= ~I2C_CR1_START;
    I2C1->SR1 |= I2C_SR1_SB;
    I2C1->SR2 |= I2C_SR2_BUSY;
    s_phase = BUS_PHASE_AWAITING_ADDR_BYTE;
}

static void handle_stop_condition(void)
{
    I2C1->CR1 &= ~I2C_CR1_STOP;
    I2C1->SR2 &= ~I2C_SR2_BUSY;
    s_phase = BUS_PHASE_IDLE;
    s_pointer_is_latched = false;
}

static void handle_address_byte(uint8_t addr_byte)
{
    uint8_t device_addr = (uint8_t)(addr_byte >> 1);
    bool read_requested  = (addr_byte & 0x01u) != 0u;

    if (device_addr != MOCK_EEPROM_SLAVE_ADDRESS)
    {
        return; /* no device answers -> ADDR flag stays clear -> driver sees a NACK */
    }

    s_current_transfer_is_read = read_requested;
    I2C1->SR1 |= I2C_SR1_ADDR;
    s_phase = BUS_PHASE_TRANSFERRING;

    if (read_requested)
    {
        /* First byte of the read is already sitting in DR by the time
         * the driver comes looking for RXNE, exactly like real EEPROMs
         * that start shifting out data right after the address ACK. */
        I2C1->DR = s_eeprom_memory[s_eeprom_pointer];
        I2C1->SR1 |= I2C_SR1_RXNE;
    }
}

static void handle_data_byte_written(uint8_t byte_value)
{
    if (!s_pointer_is_latched)
    {
        /* First byte after the address phase of a write is the memory
         * offset to operate on - this is the "internal address pointer"
         * write every byte-addressable EEPROM expects. */
        s_eeprom_pointer     = (uint8_t)(byte_value % MOCK_EEPROM_SIZE_BYTES);
        s_pointer_is_latched = true;
    }
    else
    {
        s_eeprom_memory[s_eeprom_pointer] = byte_value;
        s_eeprom_pointer = (uint8_t)((s_eeprom_pointer + 1u) % MOCK_EEPROM_SIZE_BYTES);
    }

    I2C1->SR1 |= I2C_SR1_TXE;
}

static void handle_read_service(void)
{
    /* Master already consumed the byte we put up (RXNE was cleared by
     * the driver) - shift the next one out, mimicking sequential read. */
    if ((I2C1->SR1 & I2C_SR1_RXNE) == 0u)
    {
        s_eeprom_pointer = (uint8_t)((s_eeprom_pointer + 1u) % MOCK_EEPROM_SIZE_BYTES);
        I2C1->DR = s_eeprom_memory[s_eeprom_pointer];
        I2C1->SR1 |= I2C_SR1_RXNE;
    }
}

static void service_adc_peripheral(void)
{
    if ((ADC1->CR & ADC_CR_START) != 0u)
    {
        /* The mocked sensor has no real settling/sampling time, so the
         * "conversion" is complete as soon as it is requested. */
        ADC1->CR &= ~ADC_CR_START;
        ADC1->SR |= ADC_SR_EOC;
    }
}

void platform_hw_service(void)
{
    service_adc_peripheral();

    if ((I2C1->CR1 & I2C_CR1_START) != 0u)
    {
        handle_start_condition();
        return;
    }

    if ((I2C1->CR1 & I2C_CR1_STOP) != 0u)
    {
        handle_stop_condition();
        return;
    }

    switch (s_phase)
    {
        case BUS_PHASE_AWAITING_ADDR_BYTE:
            if ((I2C1->SR1 & I2C_SR1_ADDR) == 0u)
            {
                handle_address_byte((uint8_t)I2C1->DR);
            }
            break;

        case BUS_PHASE_TRANSFERRING:
            if (s_current_transfer_is_read)
            {
                handle_read_service();
            }
            else if ((I2C1->SR1 & I2C_SR1_TXE) == 0u)
            {
                handle_data_byte_written((uint8_t)I2C1->DR);
            }
            break;

        case BUS_PHASE_IDLE:
        default:
            break;
    }
}
