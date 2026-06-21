cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico2_w)

# Include the Pico SDK initialisation script
include($ENV{PICO_SDK_PATH}/pico_sdk_init.cmake)

project(pico2w_ssd1351_libtest)

# Initialize the Pico SDK
pico_sdk_init()

# Eigene Libraries einbinden
add_subdirectory(lib/ssd1351)

# Add your executable and source files
add_executable(pico2w_ssd1351_libtest
    src/main.cpp
)

# Enable USB stdio and disable UART stdio
pico_enable_stdio_usb(pico2w_ssd1351_libtest 1)
pico_enable_stdio_uart(pico2w_ssd1351_libtest 0)

# Link the Pico SDK to your project
target_link_libraries(pico2w_ssd1351_libtest 
    pico_stdlib
    pico_cyw43_arch_none
    ssd1351
)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(pico2w_ssd1351_libtest)