#pragma once
#include <FastLED.h>
#include "config.h"

#define LED_PIN 14

class LedMatrix {
    CRGB leds[PIXELS_NUM];
public:
    LedMatrix();
    void init();
    void clear();
    void setColor();
    void animateColor();
    void displayDeparture(uint8_t timeToDeparture);
    void displayIcon(uint8_t index);
    void bootAnimation(unsigned long frame);
    void connectionAnimation(unsigned long frame);
    void sleepAnimation(unsigned long frame);
    void stressTest();
};