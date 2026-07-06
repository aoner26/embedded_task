/**
 * @file    isr_vectors.cpp
 * @brief   Interrupt service routine bodies referenced from the vector
 *          table in startup/startup_gcc.cpp.
 *
 * A real ISR has to be a plain function with C linkage - it cannot be a
 * C++ member function - so this is the standard "trampoline" pattern:
 * a free function with extern "C" linkage that forwards straight into
 * the one TemperatureMonitorApplication instance that registered
 * itself as active. The handler itself stays a one-liner; all the
 * actual work happens in TemperatureMonitorApplication::onRealTimeTick().
 *
 * On the PC build nothing ever triggers this handler - there is no
 * hardware timer counting down - but per the task requirements it is
 * still fully implemented here rather than left as a stub.
 */

#include "app/TemperatureMonitorApplication.hpp"

extern "C" void TIM2_IRQHandler(void)
{
    TemperatureMonitorApplication *activeApplication = app_isr_dispatch::activeInstance();

    if (activeApplication != nullptr)
    {
        activeApplication->onRealTimeTick();
    }
}
