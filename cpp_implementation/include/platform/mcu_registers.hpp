/**
 * @file    mcu_registers.hpp
 * @brief   Memory-mapped peripheral register layout for the target MCU.
 *
 * Same idea as a CMSIS device header: a struct of volatile registers
 * overlaid on a fixed address, so driver code reads exactly like it
 * would on real silicon. On the PC demonstration build
 * (platform_sim/mock_registers.cpp) there is obviously no physical bus
 * to overlay these structs onto, so each peripheral pointer below is
 * backed by an ordinary RAM instance instead - the driver classes never
 * need to know the difference.
 *
 * Register widths/offsets are intentionally simplified - this is not a
 * datasheet-accurate reproduction of a specific vendor part, but the
 * structure and access pattern is representative of a real Cortex-M
 * class MCU.
 */

#ifndef PLATFORM_MCU_REGISTERS_HPP
#define PLATFORM_MCU_REGISTERS_HPP

#include <cstdint>

namespace mcu
{

struct GpioRegs
{
    volatile std::uint32_t moder;  ///< Port mode register (00 = input, 01 = output)
    volatile std::uint32_t odr;    ///< Output data register
    volatile std::uint32_t idr;    ///< Input data register (read-only)
};

struct AdcRegs
{
    volatile std::uint32_t cr;     ///< Control register (enable, start conversion)
    volatile std::uint32_t sr;     ///< Status register (EOC = end of conversion)
    volatile std::uint32_t chsel;  ///< Selected input channel
    volatile std::uint32_t dr;     ///< Data register - last converted raw sample
};

constexpr std::uint32_t kAdcCrAden  = 1u << 0; ///< ADC enable
constexpr std::uint32_t kAdcCrStart = 1u << 1; ///< Start a single conversion
constexpr std::uint32_t kAdcSrEoc   = 1u << 0; ///< End-of-conversion flag

struct I2cRegs
{
    volatile std::uint32_t cr1;    ///< Control register (PE, START, STOP)
    volatile std::uint32_t sr1;    ///< Status register 1 (SB, ADDR, TXE, RXNE)
    volatile std::uint32_t sr2;    ///< Status register 2 (BUSY)
    volatile std::uint32_t dr;     ///< Data register
};

constexpr std::uint32_t kI2cCr1Pe    = 1u << 0; ///< Peripheral enable
constexpr std::uint32_t kI2cCr1Start = 1u << 1; ///< Generate (repeated) start condition
constexpr std::uint32_t kI2cCr1Stop  = 1u << 2; ///< Generate stop condition

constexpr std::uint32_t kI2cSr1Sb    = 1u << 0; ///< Start bit sent
constexpr std::uint32_t kI2cSr1Addr  = 1u << 1; ///< Address sent/matched, ACK received
constexpr std::uint32_t kI2cSr1Txe   = 1u << 2; ///< Data register empty, ready for next TX byte
constexpr std::uint32_t kI2cSr1Rxne  = 1u << 3; ///< Data register not empty, byte received

constexpr std::uint32_t kI2cSr2Busy  = 1u << 0; ///< Bus busy

struct TimRegs
{
    volatile std::uint32_t cr1;    ///< Control register (counter enable)
    volatile std::uint32_t psc;    ///< Prescaler
    volatile std::uint32_t arr;    ///< Auto-reload value (defines the period)
    volatile std::uint32_t cnt;    ///< Free running counter
    volatile std::uint32_t sr;     ///< Status register (UIF = update flag)
    volatile std::uint32_t dier;   ///< Interrupt enable register
};

constexpr std::uint32_t kTimCr1Cen   = 1u << 0; ///< Counter enable
constexpr std::uint32_t kTimSrUif    = 1u << 0; ///< Update interrupt flag
constexpr std::uint32_t kTimDierUie  = 1u << 0; ///< Update interrupt enable

/*
 * Peripheral instance pointers. On real hardware these would be literal
 * addresses from the reference manual (e.g.
 * "inline GpioRegs *const GPIOA = reinterpret_cast<GpioRegs *>(0x40010800u);").
 * platform_sim/mock_registers.cpp supplies the RAM-backed storage for
 * the PC build behind these very same symbols.
 */
extern GpioRegs *const GPIOA;
extern AdcRegs  *const ADC1;
extern I2cRegs  *const I2C1;
extern TimRegs  *const TIM2;

} // namespace mcu

#endif // PLATFORM_MCU_REGISTERS_HPP
