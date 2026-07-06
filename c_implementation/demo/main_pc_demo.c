/**
 * @file    main_pc_demo.c
 * @brief   PC-hosted demonstration of the temperature monitor firmware.
 *
 * This file is the "test bench" - it plays the role of everything that
 * would normally be soldered to the board: it pre-programs the
 * simulated EEPROM the way a factory programmer would, and it drives
 * the simulated ADC through a temperature profile the way a heat gun
 * and a cold spray can would on a real bench. Everything it calls into
 * (app_temp_monitor_*) is the same firmware that would run on target.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "app/app_temp_monitor.h"
#include "platform/mock_environment.h"
#include "service/device_config_service.h"
#include "service/temperature_service.h"

/* What the EEPROM would already contain by the time the board is
 * powered on for the very first time. */
#define DEMO_HW_REVISION    HW_REVISION_B
#define DEMO_SERIAL_NUMBER  "ABC1234"

static uint16_t deci_celsius_to_raw_count(int16_t deci_celsius)
{
    /* The test bench has to know the sensor's real transfer curve to
     * generate believable ADC codes - this mirrors the exact scale
     * factor temp_svc_init() applies for DEMO_HW_REVISION. */
    return (DEMO_HW_REVISION == HW_REVISION_B)
               ? (uint16_t)deci_celsius
               : (uint16_t)(deci_celsius / 10);
}

static const char *zone_to_string(temp_zone_t zone)
{
    switch (zone)
    {
        case TEMP_ZONE_CRITICAL_LOW:  return "RED    - critical (low)";
        case TEMP_ZONE_NORMAL:        return "GREEN  - ok";
        case TEMP_ZONE_WARNING:       return "YELLOW - warning";
        case TEMP_ZONE_CRITICAL_HIGH: return "RED    - critical (high)";
        default:                      return "UNKNOWN";
    }
}

int main(void)
{
    /* --- Program the simulated EEPROM, as the factory would -------- */
    uint8_t revision_byte = (uint8_t)DEMO_HW_REVISION;
    mock_eeprom_program(0x00u, &revision_byte, 1u);
    mock_eeprom_program(0x01u, (const uint8_t *)DEMO_SERIAL_NUMBER, sizeof(DEMO_SERIAL_NUMBER));

    printf("=== Temperature Monitor - PC demonstration (C implementation) ===\n\n");

    /* --- Board bring-up: drivers, then the one-time EEPROM read ----- */
    if (!app_temp_monitor_init())
    {
        fprintf(stderr, "FATAL: failed to read board configuration from EEPROM\n");
        return 1;
    }

    const device_config_t *config = app_temp_monitor_get_board_config();
    printf("Hardware revision : %s\n", (config->hw_revision == HW_REVISION_B) ? "Rev-B (0.1 degC/count)" : "Rev-A (1.0 degC/count)");
    printf("Serial number     : %s\n\n", config->serial_number);

    /* Only now, with the configuration cached, is it safe to let the
     * real-time task run. */
    app_temp_monitor_start();

    /*
     * Simulated temperature profile, in tenths of a degree Celsius.
     * Sweeps through every zone and deliberately loiters right on top
     * of the 5.0 / 85.0 / 105.0 degC boundaries to demonstrate that the
     * +/-1 degC hysteresis band keeps the LEDs from chattering there.
     * Each entry stands in for one 100 us tick of the real-time task.
     */
    static const int16_t temperature_profile_deci_c[] = {
        200, 400, 600, 800,           /* normal, climbing               */
        840, 845, 849,                /* nibbling the 85.0 degC edge    */
        850, 860, 900, 950, 1000,     /* into warning                   */
        1040, 1045, 1049,             /* nibbling the 105.0 degC edge   */
        1050, 1080, 1100,             /* into critical - high           */
        1049, 1041, 1035,             /* dropping back into the hysteresis band - must stay RED */
        1000, 900, 840,               /* back through warning           */
        830, 800, 600, 400,           /* back to normal                 */
        100, 60, 51,                  /* nibbling the 5.0 degC edge     */
        49, 40, 10,                   /* into critical - low            */
        51, 55, 60,                   /* rising back into the hysteresis band - must stay RED */
        100, 300, 500                 /* recovered to normal            */
    };
    const size_t sample_count = sizeof(temperature_profile_deci_c) / sizeof(temperature_profile_deci_c[0]);

    printf("%-10s %-9s %-10s %-24s\n", "tick", "raw_adc", "temp_C", "led_status");
    printf("--------------------------------------------------------\n");

    temp_zone_t previous_zone = temp_svc_get_zone();

    for (size_t tick = 0; tick < sample_count; ++tick)
    {
        int16_t deci_c = temperature_profile_deci_c[tick];
        uint16_t raw_count = deci_celsius_to_raw_count(deci_c);

        /* Stand in for the sensor driving a new voltage onto the ADC
         * pin, then run exactly the routine TIM2_IRQHandler() would run
         * on target every 100 us. */
        mock_adc_set_raw_sample(raw_count);
        app_temp_monitor_on_sample_tick();

        temp_zone_t zone = temp_svc_get_zone();
        int16_t reported_deci_c = temp_svc_get_temperature_deci_celsius();

        printf("%-10zu %-9u %+6d.%01d C  %-24s%s\n",
               tick, raw_count,
               reported_deci_c / 10, (reported_deci_c < 0 ? -reported_deci_c : reported_deci_c) % 10,
               zone_to_string(zone),
               (zone != previous_zone) ? "  <-- LED transition" : "");

        previous_zone = zone;
    }

    printf("\nDemo finished - %zu samples processed.\n", sample_count);
    return 0;
}
