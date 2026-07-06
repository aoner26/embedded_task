/**
 * @file    mock_environment.hpp
 * @brief   Test-bench style hooks used only by the PC demo harness.
 *
 * These are not part of the driver/service/application stack - they let
 * the demo program stand in for "the physical world" (a pre-programmed
 * EEPROM chip and a temperature sensor voltage sweeping across the
 * ranges of interest) so the real firmware classes can be exercised
 * without any actual hardware attached.
 */

#ifndef PLATFORM_MOCK_ENVIRONMENT_HPP
#define PLATFORM_MOCK_ENVIRONMENT_HPP

#include <cstddef>
#include <cstdint>

namespace mcu
{

/// Pre-programs the simulated EEPROM, as if it came from the factory.
void mockEepromProgram(std::uint16_t offset, const std::uint8_t *data, std::size_t length);

/// Places a new raw sample in the mock ADC's data register - stands in
/// for the sensor driving a voltage onto the ADC input pin.
void mockAdcSetRawSample(std::uint16_t rawValue);

} // namespace mcu

#endif // PLATFORM_MOCK_ENVIRONMENT_HPP
