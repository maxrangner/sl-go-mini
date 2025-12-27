#pragma once
#include "GawiButtons.h"
#include "LedMatrix.h"

class SystemManager {
    Button* _Button;
    ButtonManager _ButtonManager;
    LedMatrix Matrix;
    TaskHandle_t uiTaskHandle = nullptr;
    TaskHandle_t dataTaskHandle = nullptr;
public:
    SystemManager();
    void init();
    static void uiTask(void* pvParameters);
    static void dataTask(void* pvParameters);
};