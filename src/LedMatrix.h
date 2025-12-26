#pragma once
#include <Adafruit_NeoPixel.h>
#include "config.h"

class LedMatrix {
    Adafruit_NeoPixel pixels;
    uint8_t activePixel;
    uint8_t r;
    uint8_t g;
    uint8_t b;
public:
    LedMatrix();
    void init();
    void changeColors();
    void displayDeparture();
};