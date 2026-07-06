#include "service/led_indicator_service.h"
#include "driver/gpio_driver.h"

#include <stdbool.h>

/* Pin assignment for the three status LEDs, all on port A. */
#define LED_PIN_GREEN   5u
#define LED_PIN_YELLOW  6u
#define LED_PIN_RED     7u

void led_svc_init(void)
{
    gpio_drv_init_output(LED_PIN_GREEN);
    gpio_drv_init_output(LED_PIN_YELLOW);
    gpio_drv_init_output(LED_PIN_RED);

    /* Nothing has been measured yet at power-up, so start with every
     * LED off rather than defaulting to a (possibly wrong) green. */
    gpio_drv_write_pin(LED_PIN_GREEN, false);
    gpio_drv_write_pin(LED_PIN_YELLOW, false);
    gpio_drv_write_pin(LED_PIN_RED, false);
}

void led_svc_update(temp_zone_t zone)
{
    bool green_on  = (zone == TEMP_ZONE_NORMAL);
    bool yellow_on = (zone == TEMP_ZONE_WARNING);
    bool red_on    = (zone == TEMP_ZONE_CRITICAL_HIGH) || (zone == TEMP_ZONE_CRITICAL_LOW);

    gpio_drv_write_pin(LED_PIN_GREEN, green_on);
    gpio_drv_write_pin(LED_PIN_YELLOW, yellow_on);
    gpio_drv_write_pin(LED_PIN_RED, red_on);
}
