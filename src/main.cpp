#include "pico/stdlib.h"
#include "ssd1351.hpp"
#include "map.hpp"
#include <cmath>
#include <cstdio> // snprintf
#include "hardware/structs/rosc.h"

#define TARGET_FPS 60

struct Ball {
    float x, y;
    float vx, vy;
    int radius;
};

void update(Ball& ball) {
    ball.x += ball.vx;
    ball.y += ball.vy;

    // Wände (128x128 Display)
    if (ball.x - ball.radius < 0 || ball.x + ball.radius >= 128) ball.vx = -ball.vx;
    if (ball.y - ball.radius < 0 || ball.y + ball.radius >= 128) ball.vy = -ball.vy;
}

int main() {
    stdio_init_all();
    
    int counter = 0;

    SSD1351::Pins pins{18,  19,   17, 20, 21};
                    // sck, mosi, cs, dc, rst
    SSD1351 disp(spi0, pins);
    disp.begin();

    disp.clear(SSD1351::rgb565(0, 0, 0));
    disp.drawString(16, 0, "Hello world!", SSD1351::rgb565(40, 200, 40));
    disp.drawRect(10, 10, 50, 50, SSD1351::rgb565(255, 0, 0));
    disp.drawLine(0, 0, 127, 127, SSD1351::rgb565(0, 255, 0));
    disp.fillRect(70, 70, 30, 30, SSD1351::rgb565(0, 0, 255));
    disp.show();
    
    sleep_ms(3000);
    
    Ball ball = {63.0, 63.0, 1, 2, 8};

    while(true) {
        
        update(ball);
        
        disp.clear();
        
        for (int x = 0; x <= 127; x++) {
            for (int y = 0; y <= 127; y++) {
                disp.setPixel(x, y, SSD1351::rgb565(map(x, 0, 127, 0, 255),
                                                    map(y, 0, 127, 0, 255),
                                                    0));
            }
        }
        
        disp.fillRect(ball.x - ball.radius, ball.y - ball.radius, ball.radius * 2, ball.radius * 2, SSD1351::rgb565(0, 0, 255));
        disp.show();
        
        sleep_ms(1000.0 / TARGET_FPS);
    };
}
