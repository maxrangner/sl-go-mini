#pragma once
#include "freertos.h"
#include "WiFi.h"
#include "GawiButtons.h"
#include "LedMatrix.h"

enum class SystemState {
    NOT_CONNECTED,       // 0
    CONNECTED,           // 1
    SETUP                // 2
};

class SystemManager {
    Button* _Button;
    ButtonManager _ButtonManager;
    LedMatrix Matrix;
    SystemState _State = SystemState::NOT_CONNECTED;
    uint8_t num = 0;
    uint8_t receiveNum = 0;

    // Tasks
    TaskHandle_t systemManagerTaskHandle = nullptr;
    TaskHandle_t uiTaskHandle = nullptr;
    TaskHandle_t dataTaskHandle = nullptr;

    // Queues
    QueueHandle_t dataQueue = nullptr;
    QueueHandle_t stateQueue = nullptr;
    QueueHandle_t departureQueue = nullptr;
public:
    SystemManager();
    void init();
    static void systemManagerTask(void* pvParameters);
    static void uiTask(void* pvParameters);
    static void dataTask(void* pvParameters);
};