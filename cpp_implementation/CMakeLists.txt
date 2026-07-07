cmake_minimum_required(VERSION 3.16)
project(temperature_monitor_cpp CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

add_compile_options(-Wall -Wextra -Wshadow -Wnon-virtual-dtor -Woverloaded-virtual)

# --- Firmware sources -------------------------------------------------
# Driver (HAL), service and application layers - this is the actual
# product firmware, portable between the real target and this PC build.
# Only firmware/isr_vectors.cpp is included here; firmware/startup/* is
# real-MCU-only (vector table / linker script) - see its README.md.
add_library(temp_monitor_firmware STATIC
    src/driver/McuGpioOutput.cpp
    src/driver/McuAdcChannel.cpp
    src/driver/McuI2cBus.cpp
    src/driver/Eeprom24LC.cpp
    src/driver/McuTimer.cpp

    src/service/DeviceConfigManager.cpp
    src/service/TemperatureScalers.cpp
    src/service/TemperatureScalerFactory.cpp
    src/service/TemperatureMonitor.cpp
    src/service/LedIndicator.cpp

    src/app/TemperatureMonitorApplication.cpp

    
)

target_include_directories(temp_monitor_firmware PUBLIC
    include
    
)

# --- PC hardware mocks -------------------------------------------------
# Stands in for the actual silicon this firmware would otherwise be
# talking to. Kept as its own target so it is obvious it is not part of
# what would ship to the real board.
add_library(temp_monitor_platform_sim STATIC
    src/platform_sim/mock_registers.cpp
    src/platform_sim/platform_hooks_sim.cpp
)

target_include_directories(temp_monitor_platform_sim PUBLIC include)

# --- PC demonstration executable ---------------------------------------
add_executable(temp_monitor_pc_demo
    demo/main_pc_demo.cpp
)

target_link_libraries(temp_monitor_pc_demo PRIVATE
    temp_monitor_firmware
    temp_monitor_platform_sim
)
