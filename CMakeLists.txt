cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico2_w)

# Include the Pico SDK initialisation script
include($ENV{PICO_SDK_PATH}/pico_sdk_init.cmake)

project(pico2w_ssd1351_game)

# Initialize the Pico SDK
pico_sdk_init()

# Eigene Libraries einbinden
add_subdirectory(lib/ssd1351)
add_subdirectory(lib/picomaplib)

# Add your executable and source files
add_executable(pico2w_ssd1351_game
    src/main.cpp
)

# Enable USB stdio and disable UART stdio
pico_enable_stdio_usb(pico2w_ssd1351_game 1)
pico_enable_stdio_uart(pico2w_ssd1351_game 0)

# Link the Pico SDK to your project
target_link_libraries(pico2w_ssd1351_game 
    pico_stdlib
    pico_cyw43_arch_none
    ssd1351
    picomaplib
)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(pico2w_ssd1351_game)
