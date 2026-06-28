#include "ssd1351.hpp"
#include "font8x8_basic.h"
#include "pico/stdlib.h"
#include <cstdlib> // abs()
#include <algorithm>

SSD1351::SSD1351(spi_inst_t* spi, Pins pins, uint16_t width, uint16_t height)
    : spi_(spi), pins_(pins), width_(width), height_(height) {
    buffer_ = new uint8_t[(size_t)width_ * height_ * 2];
}

SSD1351::~SSD1351() {
    delete[] buffer_;
}

namespace {
    int min3(int a, int b, int c) { return a < b ? (a < c ? a : c) : (b < c ? b : c); }
    int max3(int a, int b, int c) { return a > b ? (a > c ? a : c) : (b > c ? b : c); }

    // positiv, wenn (x,y) links von der Kante (x1,y1)->(x2,y2) liegt, negativ wenn rechts
    int edgeFunction(int x1, int y1, int x2, int y2, int x, int y) {
        return (x2 - x1) * (y - y1) - (y2 - y1) * (x - x1);
    }
}

void SSD1351::begin(uint32_t spi_freq_hz) {
    spi_init(spi_, spi_freq_hz);
    gpio_set_function(pins_.sck, GPIO_FUNC_SPI);
    gpio_set_function(pins_.mosi, GPIO_FUNC_SPI);

    gpio_init(pins_.cs);  gpio_set_dir(pins_.cs, GPIO_OUT);  gpio_put(pins_.cs, 1);
    gpio_init(pins_.dc);  gpio_set_dir(pins_.dc, GPIO_OUT);
    gpio_init(pins_.rst); gpio_set_dir(pins_.rst, GPIO_OUT);

    reset();

    writeCommand(COMMANDLOCK, (const uint8_t[]){0x12}, 1);
    writeCommand(COMMANDLOCK, (const uint8_t[]){0xB1}, 1);
    writeCommand(DISPLAYOFF);
    writeCommand(CLOCKDIV, (const uint8_t[]){0xF1}, 1);
    writeCommand(MUXRATIO, (const uint8_t[]){127}, 1);
    writeCommand(DISPLAYOFFSET, (const uint8_t[]){0x00}, 1);
    writeCommand(SETGPIO, (const uint8_t[]){0x00}, 1);
    writeCommand(FUNCTIONSELECT, (const uint8_t[]){0x01}, 1);
    writeCommand(PRECHARGE, (const uint8_t[]){0x32}, 1);
    writeCommand(VCOMH, (const uint8_t[]){0x05}, 1);
    writeCommand(NORMALDISPLAY);
    writeCommand(CONTRASTABC, (const uint8_t[]){0xC8, 0x80, 0xC8}, 3);
    writeCommand(CONTRASTMASTER, (const uint8_t[]){0x0F}, 1);
    writeCommand(SETVSL, (const uint8_t[]){0xA0, 0xB5, 0x55}, 3);
    writeCommand(PRECHARGE2, (const uint8_t[]){0x01}, 1);

    // Remap: 64K-Farben, gesplittete COM-Zeilen, CBA-Reihenfolge, Scan bottom-up
    writeCommand(SETREMAP, (const uint8_t[]){0x74}, 1);
    writeCommand(STARTLINE, (const uint8_t[]){0x00}, 1);

    writeCommand(DISPLAYON);
    
    clear();
}

void SSD1351::clear(uint16_t color565) {
    uint8_t hi = color565 >> 8, lo = color565 & 0xFF;
    size_t n = (size_t)width_ * height_;
    for (size_t i = 0; i < n; ++i) {
        buffer_[2*i]     = hi;
        buffer_[2*i + 1] = lo;
    }
}

void SSD1351::setPixel(uint16_t x, uint16_t y, uint16_t color565) {
    if (x >= width_ || y >= height_) return;
    size_t idx = ((size_t)y * width_ + x) * 2;
    buffer_[idx]     = color565 >> 8;
    buffer_[idx + 1] = color565 & 0xFF;
}

void SSD1351::drawLine(int x1, int y1, int x2, int y2, uint16_t color565) {
    int dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
    int dy = -abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
    int err = dx + dy;
    while (true) {
        setPixel((uint16_t)x1, (uint16_t)y1, color565);
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x1 += sx; }
        if (e2 <= dx) { err += dx; y1 += sy; }
    }
}

void SSD1351::drawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color565) {
    drawLine(x, y, x + w - 1, y, color565);
    drawLine(x, y + h - 1, x + w - 1, y + h - 1, color565);
    drawLine(x, y, x, y + h - 1, color565);
    drawLine(x + w - 1, y, x + w - 1, y + h - 1, color565);
}

void SSD1351::fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color565) {
    for (uint16_t row = y; row < y + h; ++row)
        for (uint16_t col = x; col < x + w; ++col)
            setPixel(col, row, color565);
}

void SSD1351::drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, uint16_t color565) {
    drawLine(x1, y1, x2, y2, color565);
    drawLine(x2, y2, x3, y3, color565);
    drawLine(x3, y3, x1, y1, color565);
}

void SSD1351::fillTriangle(int x1, int y1, int x2, int y2, int x3, int y3, uint16_t color565) {
    int minX = std::max(0, min3(x1, x2, x3));
    int minY = std::max(0, min3(y1, y2, y3));
    int maxX = std::min(width_ - 1, max3(x1, x2, x3));
    int maxY = std::min(height_ - 1, max3(y1, y2, y3));

    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            int w0 = edgeFunction(x2, y2, x3, y3, x, y);
            int w1 = edgeFunction(x3, y3, x1, y1, x, y);
            int w2 = edgeFunction(x1, y1, x2, y2, x, y);
            if ((w0 >= 0 && w1 >= 0 && w2 >= 0) || (w0 <= 0 && w1 <= 0 && w2 <= 0)) {
                setPixel((uint16_t)x, (uint16_t)y, color565);
            }
        }
    }
}

void SSD1351::drawChar(uint16_t x, uint16_t y, char c, uint16_t color565, uint8_t scale) {
    unsigned char idx = static_cast<unsigned char>(c);
    if (idx > 127) return; // nur Basic-ASCII abgedeckt

    const uint8_t* glyph = reinterpret_cast<const uint8_t*>(font8x8_basic[idx]);

    for (int row = 0; row < 8; ++row) {
        uint8_t bits = glyph[row];
        for (int col = 0; col < 8; ++col) {
            if (bits & (1 << col)) {
                if (scale == 1) {
                    setPixel(x + col, y + row, color565);
                } else {
                    fillRect(x + col * scale, y + row * scale, scale, scale, color565);
                }
            }
        }
    }
}

void SSD1351::drawString(uint16_t x, uint16_t y, const char* s, uint16_t color565, uint8_t scale) {
    uint16_t cursorX = x;
    uint16_t cursorY = y;
    while (*s) {
        if (*s == '\n') {
            cursorY += 8 * scale + 1;
            cursorX = x;
        } else {
            drawChar(cursorX, cursorY, *s, color565, scale);
            cursorX += 8 * scale;
        }
        ++s;
    }
}

void SSD1351::drawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                        const uint16_t* pixels) {
    for (uint16_t row = 0; row < h; ++row) {
        if (y + row >= height_) break;
        for (uint16_t col = 0; col < w; ++col) {
            if (x + col >= width_) break;
            uint16_t px = pixels[row * w + col];
            if (!(px & 0x8000)) continue;  // transparent

            uint8_t r = (px >> 10) & 0x1F;
            uint8_t g = (px >>  5) & 0x1F;
            uint8_t b = (px >>  0) & 0x1F;
            setPixel(x + col, y + row, (r << 11) | (g << 6) | b);
        }
    }
}

void SSD1351::drawImageAlpha(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                             const uint8_t* pixels) {  // 3 Bytes pro Pixel
    for (uint16_t row = 0; row < h; ++row) {
        if (y + row >= height_) break;
        for (uint16_t col = 0; col < w; ++col) {
            if (x + col >= width_) break;
            size_t i = (row * w + col) * 3;
            uint8_t  a  = pixels[i];
            uint16_t px = (pixels[i+1] << 8) | pixels[i+2];

            if (a == 0) continue;  // voll transparent
            if (a == 255) {
                setPixel(x + col, y + row, px);
            } else {
                // Alpha-Blending mit Framebuffer-Pixel
                size_t   idx = ((size_t)(y + row) * width_ + (x + col)) * 2;
                uint16_t bg  = (buffer_[idx] << 8) | buffer_[idx + 1];

                uint8_t r1 = (px >> 11) & 0x1F, r2 = (bg >> 11) & 0x1F;
                uint8_t g1 = (px >>  5) & 0x3F, g2 = (bg >>  5) & 0x3F;
                uint8_t b1 =  px        & 0x1F, b2 =  bg        & 0x1F;

                uint8_t r = (r1 * a + r2 * (255 - a)) / 255;
                uint8_t g = (g1 * a + g2 * (255 - a)) / 255;
                uint8_t b = (b1 * a + b2 * (255 - a)) / 255;

                setPixel(x + col, y + row, (r << 11) | (g << 5) | b);
            }
        }
    }
}

void SSD1351::show() {
    setAddrWindow(0, 0, width_, height_);
    writeData(buffer_, (size_t)width_ * height_ * 2);
}

void SSD1351::reset() {
    gpio_put(pins_.rst, 1); sleep_ms(5);
    gpio_put(pins_.rst, 0); sleep_ms(5);
    gpio_put(pins_.rst, 1); sleep_ms(5);
}

void SSD1351::writeCommand(uint8_t cmd) { writeCommand(cmd, nullptr, 0); }

void SSD1351::writeCommand(uint8_t cmd, const uint8_t* data, size_t len) {
    gpio_put(pins_.cs, 0);
    gpio_put(pins_.dc, 0);
    spi_write_blocking(spi_, &cmd, 1);
    if (data && len) {
        gpio_put(pins_.dc, 1);
        spi_write_blocking(spi_, data, len);
    }
    gpio_put(pins_.cs, 1);
}

void SSD1351::writeData(const uint8_t* data, size_t len) {
    gpio_put(pins_.cs, 0);
    gpio_put(pins_.dc, 1);
    spi_write_blocking(spi_, data, len);
    gpio_put(pins_.cs, 1);
}

void SSD1351::setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    uint8_t col[2] = { (uint8_t)x, (uint8_t)(x + w - 1) };
    uint8_t row[2] = { (uint8_t)y, (uint8_t)(y + h - 1) };
    writeCommand(SETCOLUMN, col, 2);
    writeCommand(SETROW, row, 2);
    writeCommand(WRITERAM);
}

// alter code, unnötig, trotzdem gebackupt hier
//void SSD1351::drawPixel(uint16_t x, uint16_t y, uint16_t color565) {
//    if (x >= width_ || y >= height_) return;
//    setAddrWindow(x, y, 1, 1);
//    uint8_t buf[2] = { (uint8_t)(color565 >> 8), (uint8_t)(color565 & 0xFF) };
//    writeData(buf, 2);
//}
//
//void SSD1351::fillScreen(uint16_t color565) {
//    setAddrWindow(0, 0, width_, height_);
//    uint8_t line[128 * 2];
//    uint8_t hi = color565 >> 8, lo = color565 & 0xFF;
//    for (int i = 0; i < width_; ++i) { line[2*i] = hi; line[2*i+1] = lo; }
//
//    gpio_put(pins_.cs, 0);
//    gpio_put(pins_.dc, 1);
//    for (int y = 0; y < height_; ++y)
//        spi_write_blocking(spi_, line, width_ * 2);
//    gpio_put(pins_.cs, 1);
//}
//
//void SSD1351::pushPixels(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t* pixels_be) {
//    setAddrWindow(x, y, w, h);
//    writeData(pixels_be, (size_t)w * h * 2);
//}

void SSD1351::displayOn(bool on)    { writeCommand(on ? DISPLAYON : DISPLAYOFF); }
void SSD1351::invertDisplay(bool enabled) { writeCommand(enabled ? INVERTDISPLAY : NORMALDISPLAY); }
