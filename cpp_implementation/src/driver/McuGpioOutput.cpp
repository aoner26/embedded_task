#include "driver/McuGpioOutput.hpp"
#include "platform/mcu_registers.hpp"

namespace
{
constexpr std::uint32_t kModerBitsPerPin   = 2u;
constexpr std::uint32_t kModerFieldMask    = 0x3u;
constexpr std::uint32_t kModerOutputPattern = 0x1u;
} // namespace

McuGpioOutput::McuGpioOutput(std::uint8_t pinNumber)
    : m_pinNumber(pinNumber)
{
}

void McuGpioOutput::configure()
{
    const std::uint32_t fieldShift = static_cast<std::uint32_t>(m_pinNumber) * kModerBitsPerPin;

    mcu::GPIOA->moder &= ~(kModerFieldMask << fieldShift);
    mcu::GPIOA->moder |= (kModerOutputPattern << fieldShift);
}

void McuGpioOutput::set(bool driveHigh)
{
    const std::uint32_t pinMask = static_cast<std::uint32_t>(1u) << m_pinNumber;

    if (driveHigh)
    {
        mcu::GPIOA->odr |= pinMask;
    }
    else
    {
        mcu::GPIOA->odr &= ~pinMask;
    }
}
