#include "app/TemperatureMonitorApplication.hpp"
#include "service/TemperatureScalerFactory.hpp"

namespace
{
// TIM2 is fed from a 16 MHz peripheral clock on this board.
constexpr std::uint32_t kTimerInputClockHz = 16000000u;
constexpr std::uint32_t kSamplePeriodUs    = 100u;

TemperatureMonitorApplication *g_activeInstance = nullptr;
} // namespace

TemperatureMonitorApplication::TemperatureMonitorApplication(TemperatureMonitor  &temperatureMonitor,
                                                               LedIndicator        &ledIndicator,
                                                               DeviceConfigManager &configManager,
                                                               ITimer              &samplingTimer)
    : m_temperatureMonitor(temperatureMonitor)
    , m_ledIndicator(ledIndicator)
    , m_configManager(configManager)
    , m_samplingTimer(samplingTimer)
{
}

bool TemperatureMonitorApplication::initialize()
{
    m_ledIndicator.turnAllOff();

    /*
     * --- One-time board configuration read-out ------------------------
     *
     * This is the only place anywhere in this application that talks to
     * the EEPROM, and it happens here - once, at boot, while the
     * sampling timer is still disabled. It must stay that way: an I2C
     * transaction takes on the order of tens of microseconds even in
     * the best case, which alone would already blow most of the 100 us
     * sampling budget, and I2C bus timing is nowhere near tight enough
     * to guarantee "very low jitter" on top of that. So instead of ever
     * touching the EEPROM from the real-time task, the one piece of
     * information it changes - which scaler to use - is resolved once
     * here and handed to the temperature monitor before the timer is
     * armed below.
     */
    if (!m_configManager.loadOnce(m_boardConfig))
    {
        return false;
    }

    m_temperatureMonitor.setScaler(TemperatureScalerFactory::create(m_boardConfig.hwRevision));

    m_samplingTimer.configurePeriodicMicros(kTimerInputClockHz, kSamplePeriodUs);

    return true;
}

void TemperatureMonitorApplication::start()
{
    m_samplingTimer.start();
}

void TemperatureMonitorApplication::onRealTimeTick()
{
    m_samplingTimer.clearInterruptFlag();
    m_temperatureMonitor.onSampleTick();
    m_ledIndicator.update(m_temperatureMonitor.getZone());
}

const DeviceConfig &TemperatureMonitorApplication::boardConfig() const
{
    return m_boardConfig;
}

void TemperatureMonitorApplication::registerAsActiveInstance()
{
    g_activeInstance = this;
}

namespace app_isr_dispatch
{
TemperatureMonitorApplication *activeInstance()
{
    return g_activeInstance;
}
} // namespace app_isr_dispatch
