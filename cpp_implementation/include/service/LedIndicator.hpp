/**
 * @file    LedIndicator.hpp
 * @brief   Maps a temperature zone onto the three status LEDs.
 */

#ifndef SERVICE_LEDINDICATOR_HPP
#define SERVICE_LEDINDICATOR_HPP

#include "driver/IDigitalOutput.hpp"
#include "service/TemperatureMonitor.hpp"

class LedIndicator
{
public:
    LedIndicator(IDigitalOutput &green, IDigitalOutput &yellow, IDigitalOutput &red);

    /// Switches all three LEDs off - used once at start-up, before the
    /// first real temperature reading exists.
    void turnAllOff();

    /// Lights the single LED that corresponds to the given zone, making
    /// sure the other two are off.
    void update(TemperatureZone zone);

private:
    IDigitalOutput &m_green;
    IDigitalOutput &m_yellow;
    IDigitalOutput &m_red;
};

#endif // SERVICE_LEDINDICATOR_HPP
