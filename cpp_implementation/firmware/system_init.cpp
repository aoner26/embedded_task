#include "system_init.hpp"

extern "C" void SystemInit(void)
{
    /*
     * On real hardware this is where the core gets switched off the
     * internal RC oscillator and onto the external crystal/PLL, and
     * where the flash controller's wait-state count gets updated to
     * match the new clock speed - both have to happen before anything
     * else relies on accurate timing (in particular, before
     * McuTimer::configurePeriodicMicros() computes a prescaler from the
     * assumed timer clock).
     *
     * The clock tree for this exercise's target is out of scope, so
     * this is deliberately left as a placeholder: everything above this
     * layer only cares about the resulting peripheral clock frequency,
     * which is passed explicitly into the timer driver instead of being
     * queried back out of here.
     */
}
