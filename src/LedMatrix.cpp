#include "LedMatrix.h"


LedMatrix::LedMatrix() : pixels(PIXELS_NUM, LED_PIN) {
    activePixel = 0;
    r = 10;
    g = 0;
    b = 0;
}

void LedMatrix::init() {
    Serial.println("LedMatrix init()");
    pixels.begin();
}

void LedMatrix::changeColors() {
    if (r == 10) {
        r = 0;
        g = 10;
    }
    else if (g == 10) {
        g = 0;
        b = 10;
    }
    else if (b = 10) {
        b = 0;
        r = 10;
    }
}

void LedMatrix::displayDeparture() {
    Serial.println("LedMatrix displayDeparture()");
    pixels.clear();
    pixels.setPixelColor(activePixel++, pixels.Color(r, g, b));
    Serial.print(r); Serial.print(g); Serial.println(b);
    pixels.show();
}
