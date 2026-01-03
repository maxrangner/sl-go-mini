#include "LedMatrix.h"

LedMatrix::LedMatrix() : pixels(PIXELS_NUM, LED_PIN) {
}

void LedMatrix::init() {
    Serial.println("LedMatrix init()");
    pixels.begin();
}

uint32_t LedMatrix::setColors(const uint8_t c[3]) {
    return pixels.Color(c[0], c[1], c[2]);
}

void LedMatrix::displayDeparture(char* rawTime) {
    pixels.clear();
    uint8_t departureTime = atoi(rawTime);
    const uint8_t* departure = numbers[departureTime];
    uint8_t* nextColor = nullptr;
    for (uint8_t i = 0; i < PIXELS_NUM; i++) {
        if (departure[i] == 1) nextColor = green;
        else nextColor = off;
        pixels.setPixelColor(i, setColors(nextColor)); // G, R, B
    }
    pixels.show();
}

void LedMatrix::clearDisplay() {
    pixels.clear();
    pixels.show();
}
