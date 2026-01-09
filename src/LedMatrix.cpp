#include "LedMatrix.h"
#include "graphics.h"
#include "animations.h"

LedMatrix::LedMatrix() {
}

void LedMatrix::init() {
    Serial.println("FastLED init()");
    FastLED.addLeds<WS2812B, LED_PIN, RGB>(leds, PIXELS_NUM);
    FastLED.clear();
    FastLED.show();
}

void LedMatrix::clear() {
    FastLED.clear();
    FastLED.show();
}

void LedMatrix::setColor() {
    // Set color
}

void LedMatrix::animateColor() {
    // Animate color
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

void LedMatrix::displayIcon(uint8_t index) {
    const uint8_t* departure = icons[index];
    
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

void LedMatrix::bootAnimation(unsigned long frame) {
    const uint8_t* animationFrame = animations[frame % 8 + 2];
    
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

void LedMatrix::connectionAnimation(unsigned long frame) {
    uint8_t holdFrame = 1;
    uint8_t numFrames = 4;
    const uint8_t* animationFrame = animations[((frame / holdFrame) % numFrames) + 10]; // + 10 is position of animation in array
    
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

void LedMatrix::sleepAnimation(unsigned long frame) {
    uint8_t holdFrame = 20;
    uint8_t numFrames = 2;
    const uint8_t* animationFrame = animations[(frame / holdFrame) % numFrames];
    
    for (uint8_t i = 0; i < PIXELS_NUM; i++) {
        if (animationFrame[i] == 1) {
            leds[i].r = 1;
            leds[i].g = 1;
            leds[i].b = 1;
        } else {
            leds[i].r = 0;
            leds[i].g = 0;
            leds[i].b = 0;
        }
    }
    FastLED.show();
}

// Scrolling graphics
// logicalX = x + scrollOffset
// pos = logicalX / (DIGIT_WIDTH + SPACING)
// digitX = logicalX % (DIGIT_WIDTH + SPACING)
// if (digitX >= DIGIT_WIDTH) → pixel off
// digitValue = digits[pos]   // t.ex 2, 2, 4, 7
// numbers[digitIndex][row * 8 + digitX]

// srcCol = (col + offset) % WIDTH;
// srcIndex = row * WIDTH + srcCol;


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