#pragma once
#include <Arduino.h>

namespace Utils {
    void writeCharArray(char* target, size_t targetSize, const char* source) {
        if (targetSize == 0 || source == nullptr) return;
    
        target[0] = '\0';
        size_t i = 0;
        for (i = 0; i < targetSize - 1 && source[i] != '\0'; i++) {
            target[i] = source[i];
        }
        target[i] = '\0';
    }
}