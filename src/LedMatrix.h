#pragma once
#include <Adafruit_NeoPixel.h>
#include "config.h"
#include "graphics.h"

class LedMatrix {
    Adafruit_NeoPixel pixels;
    uint8_t red[3] = {0, 5, 0};
    uint8_t green[3] = {5, 0, 0};
    uint8_t blue[3] = {0, 0, 5};
    uint8_t off[3] = {0, 0, 0};
    uint32_t setColors(const uint8_t c[3]);
public:
    LedMatrix();
    void init();
    void displayDeparture(char* time);
    void clearDisplay();
};