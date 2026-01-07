#include "LedMatrix.h"

LedMatrix::LedMatrix() {
}

void LedMatrix::init() {
    FastLED.addLeds<WS2812B, LED_PIN, RGB>(leds, PIXELS_NUM);
    FastLED.clear();
    FastLED.show();
}

void LedMatrix::clear() {
    FastLED.clear();
    FastLED.show();
}

void LedMatrix::displayConnecting(unsigned long frame) {
    // // Frame rate debug
    // static unsigned long lastFrame = 0;
    // unsigned long now = millis();
    // printf("Frame %lu, delta: %lu ms\n", frame % 4, now - lastFrame);
    // lastFrame = now;

    const uint8_t* animationFrame = icons[frame % 4 + 21];
    
    for (uint8_t i = 0; i < PIXELS_NUM; i++) {
        if (animationFrame[i] == 1) {
            leds[i].r = frame % 6 ;
            leds[i].g = (frame + 2) % 6;
            leds[i].b = (frame + 4) % 6;
        } else {
            leds[i].r = 0;
            leds[i].g = 0;
            leds[i].b = 0;
        }
    }
    FastLED.show();
}

void LedMatrix::displayDeparture(uint8_t timeToDeparture) {
    const uint8_t* departure = numbers[timeToDeparture];
    
    for (uint8_t i = 0; i < PIXELS_NUM; i++) {
        if (departure[i] == 1) {
            leds[i].r = 0;
            leds[i].g = 1;
            leds[i].b = 0;
        } else {
            leds[i].r = 0;
            leds[i].g = 0;
            leds[i].b = 0;
        }
    }
    FastLED.show();
}

void LedMatrix::stressTest() {
    Serial.println("\n=== LED STRESSTEST START (FastLED) ===");
    Serial.println("Uppdaterar pixels.show() 1000 gånger...");
    Serial.println("Loggar ENDAST verkliga timing-spikes (>6ms)");
    Serial.println("------------------------------------------------");
    
    const uint32_t SPIKE_THRESHOLD_US = 6000;
    uint32_t maxShow = 0;
    uint32_t spikeCount = 0;
    uint32_t totalCalls = 0;
    
    for (int i = 0; i < 1000; i++) {
        uint32_t start = micros();
        displayDeparture(1);
        uint32_t dt = micros() - start;
        
        totalCalls++;
        
        if (dt > maxShow) {
            maxShow = dt;
        }
        
        if (dt > SPIKE_THRESHOLD_US) {
            spikeCount++;
            Serial.printf(
                "[SPIKE] iter=%d | show=%lu µs | heap=%u\n",
                i,
                dt,
                ESP.getFreeHeap()
            );
        }
        
        if ((i + 1) % 100 == 0) {
            Serial.printf(
                "[STATS] %d calls | spikes=%lu | max=%lu µs\n",
                i + 1,
                spikeCount,
                maxShow
            );
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    Serial.println("------------------------------------------------");
    Serial.println("=== STRESSTEST KLAR ===");
    Serial.printf("Totalt calls: %lu\n", totalCalls);
    Serial.printf("Totala spikes (>6ms): %lu\n", spikeCount);
    Serial.printf("Max show(): %lu µs\n", maxShow);
    Serial.println("Observera: Visuella glitches ska matcha SPIKE-loggar.");
}