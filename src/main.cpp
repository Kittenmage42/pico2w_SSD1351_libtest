#include "pico/stdlib.h"
#include "ssd1351.hpp"
#include <cmath>
#include <cstdio> // snprintf
#include "hardware/structs/rosc.h"

#define TARGET_FPS 60

int map(int x, int in_min, int in_max,
        int out_min, int out_max)
{
    return (x - in_min) * (out_max - out_min)
           / (in_max - in_min)
           + out_min;
}

float mapf(float x,
           float in_min, float in_max,
           float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min)
           / (in_max - in_min)
           + out_min;
}


int main() {
    stdio_init_all();
    
    int counter = 0;

    SSD1351::Pins pins{18, 19, 17, 20, 21};
    SSD1351 disp(spi0, pins);
    disp.begin();

    disp.clear(SSD1351::rgb565(0, 0, 0));
    disp.drawString(16, 0, "Hello world!", SSD1351::rgb565(40, 200, 40));
    disp.drawRect(10, 10, 50, 50, SSD1351::rgb565(255, 0, 0));
    disp.drawLine(0, 0, 127, 127, SSD1351::rgb565(0, 255, 0));
    disp.fillRect(70, 70, 30, 30, SSD1351::rgb565(0, 0, 255));
    disp.show();
    
    sleep_ms(3000);
    
    disp.clear(SSD1351::rgb565(0, 0, 0));
    
    for (int x = 0; x <= 127; x++) {
        for (int y = 0; y <= 127; y++) {
            disp.setPixel(x, y, SSD1351::rgb565(map(x, 0, 127, 0, 255),
                                                map(y, 0, 127, 0, 255),
                                                0));
        }
    }
    
    disp.show();

    while (true) tight_loop_contents();
}
