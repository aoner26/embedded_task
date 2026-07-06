#include "service/device_config_service.h"
#include "driver/eeprom_driver.h"

/* EEPROM memory map - this is the one place in the whole project that
 * needs to know where things live in the configuration EEPROM. */
#define EEPROM_OFFSET_HW_REVISION    0x00u
#define EEPROM_OFFSET_SERIAL_NUMBER  0x01u
#define SERIAL_NUMBER_STORED_LEN     (DEVICE_SERIAL_NUMBER_MAX_LEN - 1u)

bool cfg_svc_load_from_eeprom(device_config_t *out_config)
{
    uint8_t raw_revision_byte = 0u;

    if (!eeprom_drv_read(EEPROM_OFFSET_HW_REVISION, &raw_revision_byte, 1u))
    {
        return false;
    }

    device_config_t loaded_config;
    loaded_config.hw_revision = (raw_revision_byte == (uint8_t)HW_REVISION_B) ? HW_REVISION_B : HW_REVISION_A;

    if (!eeprom_drv_read(EEPROM_OFFSET_SERIAL_NUMBER,
                          (uint8_t *)loaded_config.serial_number,
                          SERIAL_NUMBER_STORED_LEN))
    {
        return false;
    }
    loaded_config.serial_number[SERIAL_NUMBER_STORED_LEN] = '\0';

    *out_config = loaded_config;
    return true;
}
