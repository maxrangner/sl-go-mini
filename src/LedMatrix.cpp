#include "LedMatrix.h"
#include "WiFi.h"

uint8_t testHeart[64] = {
    0,0,0,0,1,0,0,0,
    0,0,0,1,0,1,0,0,
    0,0,1,0,0,0,1,0,
    0,1,0,0,0,0,0,1,
    0,1,0,0,1,0,0,1,
    0,0,1,1,0,1,1,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0
};

LedMatrix::LedMatrix() : pixels(PIXELS_NUM, LED_PIN) {
}

void LedMatrix::init() {
    Serial.println("LedMatrix init()");
    pixels.begin();
}

void LedMatrix::displayDeparture(uint8_t timeToDeparture) {
    pixels.clear();
    const uint8_t* departure = numbers[timeToDeparture];
    // const uint8_t* departure = num_3;  
    uint32_t nextColor;
    for (uint8_t i = 0; i < PIXELS_NUM; i++) {
        uint8_t pixel = departure[i];
        if (pixel == 1) pixels.setPixelColor(i, 5, 0, 0);
    }
    pixels.show();
}

void LedMatrix::clearDisplay() {
    pixels.clear();
    pixels.show();
}
