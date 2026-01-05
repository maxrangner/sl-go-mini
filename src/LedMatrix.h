#pragma once
#include <FastLED.h>
#include "config.h"
#include "graphics.h"

#define LED_PIN 14

class LedMatrix {
    CRGB leds[PIXELS_NUM];
public:
    LedMatrix();
    void init();
    void displayDeparture(uint8_t timeToDeparture);
    void stressTest();
};