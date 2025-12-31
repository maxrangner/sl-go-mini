#pragma once
#include "freertos.h"
#include "GawiButtons.h"
#include "LedMatrix.h"
#include "NetworkManager.h"
#include "utils.h"

enum class SystemState {
    BOOT,
    CONNECTING,
    CONNECTED,           // 1
    DISCONNECTED,
    SETUP                // 2
};

class SystemManager {
    // Class instanses
    NetworkManager networkManager;
    Button* mainButton;
    ButtonManager buttonMng;
    LedMatrix matrix;

    // Variables
    SystemState state = SystemState::BOOT;
    unsigned long now;
    unsigned long prevTime;
    uint8_t receiveNum = 0;
    QueueMessage receivedData;

    // Tasks
    TaskHandle_t systemTaskHandle = nullptr;
    TaskHandle_t networkTaskHandle = nullptr;

    // Queues
    QueueHandle_t dataQueue = nullptr;
public:
    SystemManager();
    void init();
    void run();
    static void systemTask(void* pvParameters);
    static void networkTask(void* pvParameters);
};