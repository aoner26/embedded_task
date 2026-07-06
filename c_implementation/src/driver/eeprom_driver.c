#include "driver/eeprom_driver.h"
#include "driver/i2c_driver.h"

void eeprom_drv_init(void)
{
    i2c_drv_init();
}

bool eeprom_drv_read(uint8_t mem_offset, uint8_t *data, size_t len)
{
    return i2c_drv_write_then_read(EEPROM_DRV_SLAVE_ADDRESS, &mem_offset, 1u, data, len);
}
