#include "app/app_temp_monitor.h"

#include "driver/adc_driver.h"
#include "driver/eeprom_driver.h"
#include "driver/timer_driver.h"

#include "service/device_config_service.h"
#include "service/led_indicator_service.h"
#include "service/temperature_service.h"

#define ADC_CHANNEL_TEMPERATURE_SENSOR   0u   /* PA0 */

/* TIM2 is fed from a 16 MHz peripheral clock on this board. */
#define TIMER_INPUT_CLOCK_HZ              16000000u
#define SAMPLE_PERIOD_US                  100u

static device_config_t g_board_config;

bool app_temp_monitor_init(void)
{
    /* --- Bring up every driver first --------------------------------- */
    adc_drv_init(ADC_CHANNEL_TEMPERATURE_SENSOR);
    eeprom_drv_init();
    led_svc_init();

    /*
     * --- One-time board configuration read-out ------------------------
     *
     * This is the only place anywhere in this application that talks to
     * the EEPROM, and it happens here - once, at boot, while the
     * sampling timer is still disabled. It must stay that way: an I2C
     * transaction takes on the order of tens of microseconds even in
     * the best case, which alone would already blow most of the 100 us
     * sampling budget, and I2C bus timing is not nearly tight enough to
     * guarantee "very low jitter" on top of that. So instead of ever
     * touching the EEPROM from the real-time task, the two pieces of
     * information we need from it (sensor scale factor and serial
     * number) are read exactly once and cached before the timer is
     * armed below.
     */
    if (!cfg_svc_load_from_eeprom(&g_board_config))
    {
        return false;
    }

    temp_svc_init(g_board_config.hw_revision);

    /* --- Arm, but do not yet start, the periodic sampling timer ------- */
    tmr_drv_configure_periodic_us(TIMER_INPUT_CLOCK_HZ, SAMPLE_PERIOD_US);

    return true;
}

void app_temp_monitor_start(void)
{
    tmr_drv_start();
}

void app_temp_monitor_on_sample_tick(void)
{
    uint16_t raw_sample = adc_drv_read_raw();

    temp_svc_process_sample(raw_sample);
    led_svc_update(temp_svc_get_zone());
}

const device_config_t *app_temp_monitor_get_board_config(void)
{
    return &g_board_config;
}
