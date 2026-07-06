/**
 * @file    TemperatureMonitorApplication.hpp
 * @brief   Wires the services together and owns the start-up sequencing.
 *
 * Everything this class needs is handed in through the constructor
 * (dependency injection) - it never constructs a driver or a service
 * itself, which is what keeps it trivially testable against fakes and
 * completely unaware of which concrete hardware it is running on.
 */

#ifndef APP_TEMPERATUREMONITORAPPLICATION_HPP
#define APP_TEMPERATUREMONITORAPPLICATION_HPP

#include "driver/ITimer.hpp"
#include "service/DeviceConfig.hpp"
#include "service/DeviceConfigManager.hpp"
#include "service/LedIndicator.hpp"
#include "service/TemperatureMonitor.hpp"

class TemperatureMonitorApplication
{
public:
    TemperatureMonitorApplication(TemperatureMonitor    &temperatureMonitor,
                                   LedIndicator          &ledIndicator,
                                   DeviceConfigManager   &configManager,
                                   ITimer                &samplingTimer);

    /**
     * @brief One-time system bring-up.
     *
     * Performs the single blocking EEPROM read that is needed to know
     * which sensor is fitted, injects the matching scaler strategy into
     * the temperature monitor, and arms the sampling timer (without
     * starting it). Must complete before start() is called - see the
     * .cpp file for why the EEPROM access in particular can never be
     * deferred into the real-time task.
     */
    bool initialize();

    /// Starts the periodic sampling timer - from this point on,
    /// onRealTimeTick() runs every 100 us via the timer's interrupt.
    void start();

    /**
     * @brief Body of the real-time sampling task.
     *
     * Clears the timer's pending interrupt flag, then updates the
     * temperature zone and the LEDs accordingly. This is what
     * TIM2_IRQHandler() calls on every tick - kept as a plain member
     * function (rather than being written inline inside the ISR) so it
     * can also be driven directly, as the PC demonstration program does.
     */
    void onRealTimeTick();

    const DeviceConfig &boardConfig() const;

    /// Registers this instance as the target of the global ISR
    /// trampoline - see firmware/isr_vectors.cpp.
    void registerAsActiveInstance();

private:
    TemperatureMonitor  &m_temperatureMonitor;
    LedIndicator         &m_ledIndicator;
    DeviceConfigManager  &m_configManager;
    ITimer               &m_samplingTimer;
    DeviceConfig          m_boardConfig{};
};

/// Seam used only by firmware/isr_vectors.cpp to reach whichever
/// application instance called registerAsActiveInstance().
namespace app_isr_dispatch
{
TemperatureMonitorApplication *activeInstance();
} // namespace app_isr_dispatch

#endif // APP_TEMPERATUREMONITORAPPLICATION_HPP
