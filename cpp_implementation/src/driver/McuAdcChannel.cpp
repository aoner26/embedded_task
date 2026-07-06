#include "driver/McuAdcChannel.hpp"
#include "platform/mcu_registers.hpp"
#include "platform/platform_hooks.hpp"

McuAdcChannel::McuAdcChannel(std::uint8_t channelNumber)
    : m_channelNumber(channelNumber)
{
}

void McuAdcChannel::enable()
{
    mcu::ADC1->chsel = m_channelNumber;
    mcu::ADC1->cr |= mcu::kAdcCrAden;
}

std::uint16_t McuAdcChannel::readRaw()
{
    mcu::ADC1->cr |= mcu::kAdcCrStart;
    mcu::hwService();

    // Blocking poll for end-of-conversion. On real silicon this takes a
    // handful of ADC clock cycles; on the PC build platform_hooks_sim.cpp
    // resolves it "instantly" inside hwService(), so this loop body never
    // actually spins here, but it is left in place because that is
    // exactly how this has to behave once running on target.
    while ((mcu::ADC1->sr & mcu::kAdcSrEoc) == 0u)
    {
        mcu::hwService();
    }

    mcu::ADC1->sr &= ~mcu::kAdcSrEoc;

    return static_cast<std::uint16_t>(mcu::ADC1->dr);
}
