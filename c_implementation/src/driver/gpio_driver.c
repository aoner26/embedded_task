#include "driver/gpio_driver.h"
#include "platform/mcu_registers.h"

/* Two bits per pin in MODER, "01" pattern means general purpose output. */
#define GPIO_MODER_BITS_PER_PIN     2u
#define GPIO_MODER_OUTPUT_PATTERN   0x1u
#define GPIO_MODER_FIELD_MASK       0x3u

void gpio_drv_init_output(uint8_t pin_no)
{
    uint32_t field_shift = (uint32_t)pin_no * GPIO_MODER_BITS_PER_PIN;

    GPIOA->MODER &= ~(GPIO_MODER_FIELD_MASK << field_shift);
    GPIOA->MODER |= (GPIO_MODER_OUTPUT_PATTERN << field_shift);
}

void gpio_drv_write_pin(uint8_t pin_no, bool drive_high)
{
    if (drive_high)
    {
        GPIOA->ODR |= (uint32_t)(1u << pin_no);
    }
    else
    {
        GPIOA->ODR &= ~(uint32_t)(1u << pin_no);
    }
}
