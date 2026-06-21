#pragma once

#include <cstdint>
#include <cstddef>
#include "hardware/spi.h"

class SSD1351 {
public:
    struct Pins { uint8_t sck, mosi, cs, dc, rst; };

    SSD1351(spi_inst_t* spi, Pins pins, uint16_t width = 128, uint16_t height = 128);
    ~SSD1351();

    // besitzt einen rohen Heap-Buffer -> nicht kopierbar
    SSD1351(const SSD1351&) = delete;
    SSD1351& operator=(const SSD1351&) = delete;

    void begin(uint32_t spi_freq_hz = 10'000'000);

    // --- wirken nur im Framebuffer (RAM) ---
    void clear(uint16_t color565 = 0x0000);
    void setPixel(uint16_t x, uint16_t y, uint16_t color565);
    void drawLine(int x1, int y1, int x2, int y2, uint16_t color565);
    void drawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color565);
    void fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color565);
    void drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, uint16_t color565);
    void fillTriangle(int x1, int y1, int x2, int y2, int x3, int y3, uint16_t color565);
    void drawChar(uint16_t x, uint16_t y, char c, uint16_t color565, uint8_t scale = 1);
    void drawString(uint16_t x, uint16_t y, const char* s, uint16_t color565, uint8_t scale = 1);

    // --- Buffer ans Display senden ---
    void show();

    void displayOn(bool on);
    void invertDisplay(bool enabled);

    uint16_t width()  const { return width_; }
    uint16_t height() const { return height_; }

    static constexpr uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
        return static_cast<uint16_t>(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
    }

private:
    enum Cmd : uint8_t {
        SETCOLUMN = 0x15, SETROW = 0x75, WRITERAM = 0x5C,
        SETREMAP = 0xA0, STARTLINE = 0xA1, DISPLAYOFFSET = 0xA2,
        NORMALDISPLAY = 0xA6, INVERTDISPLAY = 0xA7, FUNCTIONSELECT = 0xAB,
        DISPLAYOFF = 0xAE, DISPLAYON = 0xAF,
        PRECHARGE = 0xB1, CLOCKDIV = 0xB3, SETVSL = 0xB4,
        SETGPIO = 0xB5, PRECHARGE2 = 0xB6, VCOMH = 0xBE,
        CONTRASTABC = 0xC1, CONTRASTMASTER = 0xC7, MUXRATIO = 0xCA,
        COMMANDLOCK = 0xFD,
    };

    spi_inst_t* spi_;
    Pins pins_;
    uint16_t width_, height_;
    uint8_t* buffer_; // width*height*2 Bytes, big-endian RGB565

    void reset();
    void writeCommand(uint8_t cmd);
    void writeCommand(uint8_t cmd, const uint8_t* data, size_t len);
    void writeData(const uint8_t* data, size_t len);
    void setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
};
