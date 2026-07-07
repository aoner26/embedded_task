cmake_minimum_required(VERSION 3.16)
project(temperature_monitor_c C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

add_compile_options(-Wall -Wextra -Wconversion -Wshadow)

# --- Firmware sources -------------------------------------------------
# Driver (HAL), service and application layers - this is the actual
# product firmware, portable between the real target and this PC build.
# Only firmware/isr_vectors.c is included here; firmware/startup/* is
# real-MCU-only (vector table / linker script) and is not part of a
# PC executable - see firmware/startup/README.md.
add_library(temp_monitor_firmware STATIC
    src/driver/gpio_driver.c
    src/driver/adc_driver.c
    src/driver/i2c_driver.c
    src/driver/eeprom_driver.c
    src/driver/timer_driver.c

    src/service/device_config_service.c
    src/service/temperature_service.c
    src/service/led_indicator_service.c

    src/app/app_temp_monitor.c

)

target_include_directories(temp_monitor_firmware PUBLIC
    include
    
)

# --- PC hardware mocks -------------------------------------------------
# Stands in for the actual silicon this firmware would otherwise be
# talking to. Kept as its own target so it is obvious it is not part of
# what would ship to the real board.
add_library(temp_monitor_platform_sim STATIC
    src/platform_sim/mock_registers.c
    src/platform_sim/platform_hooks_sim.c
)

target_include_directories(temp_monitor_platform_sim PUBLIC include)

# --- PC demonstration executable ---------------------------------------
add_executable(temp_monitor_pc_demo
    demo/main_pc_demo.c
)

target_link_libraries(temp_monitor_pc_demo PRIVATE
    temp_monitor_firmware
    temp_monitor_platform_sim
)
