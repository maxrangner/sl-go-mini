#pragma once
#include <Arduino.h>
#include "NetworkManager.h"

namespace Utils {
    inline void writeCharArray(char* target, size_t targetSize, const char* source) {
        if (targetSize == 0 || source == nullptr) return;
    
        target[0] = '\0';
        size_t i;
        for (i = 0; i < targetSize - 1 && source[i] != '\0'; i++) {
            target[i] = source[i];
        }
        target[i] = '\0';
    }

    inline void convertTexttoMinutes(char* target, size_t targetSize, const char* source) {
        size_t i;
        for (i = 0; source[i] != '\0' && i < targetSize - 1; i++) {
            if (source[i] == ' ') break;
            target[i] = source[i];
            
        }
        target[i] = '\0';
    }
}

