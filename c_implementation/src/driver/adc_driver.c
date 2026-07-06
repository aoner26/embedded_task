#include "driver/adc_driver.h"
#include "platform/mcu_registers.h"
#include "platform/platform_hooks.h"

void adc_drv_init(uint8_t channel_no)
{
    ADC1->CHSEL = channel_no;
    ADC1->CR |= ADC_CR_ADEN;
}

uint16_t adc_drv_read_raw(void)
{
    ADC1->CR |= ADC_CR_START;
    platform_hw_service();

    /* Blocking poll for end-of-conversion. On the real part this takes a
     * handful of ADC clock cycles - well under the 100 us sample period.
     * In the PC simulation the mock peripheral resolves this "instantly"
     * inside platform_hw_service(), so the loop body below never actually
     * has to spin, but it is left in place because that is exactly how
     * this function has to behave once it is running on target. */
    while ((ADC1->SR & ADC_SR_EOC) == 0u)
    {
        platform_hw_service();
    }

    ADC1->SR &= ~ADC_SR_EOC;

    return (uint16_t)ADC1->DR;
}
