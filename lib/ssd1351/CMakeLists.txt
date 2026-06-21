add_library(ssd1351 STATIC
    ssd1351.cpp
)

target_include_directories(ssd1351 PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(ssd1351 PUBLIC
    pico_stdlib
    hardware_spi
)