#pragma once
#include "freertos.h"
#include "WiFi.h"
#include "GawiButtons.h"
#include "LedMatrix.h"
#include "NetworkManager.h"

enum class SystemState {
    NOT_CONNECTED,       // 0
    CONNECTED,           // 1
    SETUP                // 2
};

class SystemManager {
    NetworkManager NetworkMng;
    Button* mainButton;
    ButtonManager ButtonMng;
    LedMatrix Matrix;
    SystemState state = SystemState::NOT_CONNECTED;
    uint8_t receiveNum = 0;

    // Tasks
    TaskHandle_t systemUiTaskHandle = nullptr;
    TaskHandle_t networkTaskHandle = nullptr;

    // Queues
    QueueHandle_t dataQueue = nullptr;
public:
    SystemManager();
    void init();
    void wifiInit();
    void run();
    static void systemManagerTask(void* pvParameters);
    static void systemUiTask(void* pvParameters);
    static void networkTask(void* pvParameters);
};