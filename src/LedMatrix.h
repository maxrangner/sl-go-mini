#pragma once
#include <Adafruit_NeoPixel.h>
#include "config.h"
#include "graphics.h"

class LedMatrix {
    Adafruit_NeoPixel pixels;
    uint32_t red;
    uint32_t green;
    uint32_t blue;
    uint32_t off;
public:
    LedMatrix();
    void init();
    void displayDeparture(uint8_t timeToDeparture);
    void clearDisplay();
};