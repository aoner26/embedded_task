#include "service/temperature_service.h"

/* Thresholds from the spec, expressed in tenths of a degree so the
 * comparisons below stay pure integer arithmetic. */
#define TEMP_WARNING_THRESHOLD_DECI_C        850    /*  85.0 degC */
#define TEMP_CRITICAL_HIGH_THRESHOLD_DECI_C 1050    /* 105.0 degC */
#define TEMP_CRITICAL_LOW_THRESHOLD_DECI_C    50    /*   5.0 degC */

/* +/- 1 degC of hysteresis on every transition, per the requirement -
 * keeps a reading sitting right on a boundary from making the LEDs
 * flicker back and forth. */
#define TEMP_HYSTERESIS_TOLERANCE_DECI_C      10

static int16_t deci_c_per_adc_count = 10;  /* Rev-A scale assumed until temp_svc_init() runs */

/* Shared between the real-time sampling task and whatever background
 * code wants to read the current status (e.g. a display or logger) -
 * marked volatile so the compiler never caches a stale copy across
 * that boundary. */
static volatile temp_zone_t current_zone           = TEMP_ZONE_NORMAL;
static volatile int16_t     last_temperature_deci_c = 0;

void temp_svc_init(hw_revision_t hw_revision)
{
    deci_c_per_adc_count = (hw_revision == HW_REVISION_B) ? 1 : 10;
    current_zone = TEMP_ZONE_NORMAL;
}

/* Pure function: given where we currently are and the newest reading,
 * decide where we should be. Kept separate from temp_svc_process_sample()
 * so the hysteresis rules can be read (and unit tested) on their own. */
static temp_zone_t evaluate_next_zone(temp_zone_t zone, int16_t temperature_deci_c)
{
    switch (zone)
    {
        case TEMP_ZONE_CRITICAL_LOW:
            if (temperature_deci_c >= (TEMP_CRITICAL_LOW_THRESHOLD_DECI_C + TEMP_HYSTERESIS_TOLERANCE_DECI_C))
            {
                return TEMP_ZONE_NORMAL;
            }
            return TEMP_ZONE_CRITICAL_LOW;

        case TEMP_ZONE_WARNING:
            if (temperature_deci_c >= TEMP_CRITICAL_HIGH_THRESHOLD_DECI_C)
            {
                return TEMP_ZONE_CRITICAL_HIGH;
            }
            if (temperature_deci_c < (TEMP_WARNING_THRESHOLD_DECI_C - TEMP_HYSTERESIS_TOLERANCE_DECI_C))
            {
                return TEMP_ZONE_NORMAL;
            }
            return TEMP_ZONE_WARNING;

        case TEMP_ZONE_CRITICAL_HIGH:
            if (temperature_deci_c < (TEMP_CRITICAL_HIGH_THRESHOLD_DECI_C - TEMP_HYSTERESIS_TOLERANCE_DECI_C))
            {
                return TEMP_ZONE_WARNING;
            }
            return TEMP_ZONE_CRITICAL_HIGH;

        case TEMP_ZONE_NORMAL:
        default:
            if (temperature_deci_c >= TEMP_WARNING_THRESHOLD_DECI_C)
            {
                return TEMP_ZONE_WARNING;
            }
            if (temperature_deci_c < TEMP_CRITICAL_LOW_THRESHOLD_DECI_C)
            {
                return TEMP_ZONE_CRITICAL_LOW;
            }
            return TEMP_ZONE_NORMAL;
    }
}

void temp_svc_process_sample(uint16_t raw_adc_sample)
{
    int16_t temperature_deci_c = (int16_t)((int32_t)raw_adc_sample * deci_c_per_adc_count);

    current_zone = evaluate_next_zone(current_zone, temperature_deci_c);
    last_temperature_deci_c = temperature_deci_c;
}

temp_zone_t temp_svc_get_zone(void)
{
    return current_zone;
}

int16_t temp_svc_get_temperature_deci_celsius(void)
{
    return last_temperature_deci_c;
}
