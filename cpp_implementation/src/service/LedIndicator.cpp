#include "service/LedIndicator.hpp"

LedIndicator::LedIndicator(IDigitalOutput &green, IDigitalOutput &yellow, IDigitalOutput &red)
    : m_green(green)
    , m_yellow(yellow)
    , m_red(red)
{
}

void LedIndicator::turnAllOff()
{
    m_green.set(false);
    m_yellow.set(false);
    m_red.set(false);
}

void LedIndicator::update(TemperatureZone zone)
{
    const bool greenOn  = (zone == TemperatureZone::Normal);
    const bool yellowOn = (zone == TemperatureZone::Warning);
    const bool redOn    = (zone == TemperatureZone::CriticalHigh) || (zone == TemperatureZone::CriticalLow);

    m_green.set(greenOn);
    m_yellow.set(yellowOn);
    m_red.set(redOn);
}
