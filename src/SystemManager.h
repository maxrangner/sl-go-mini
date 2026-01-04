#pragma once
#include "freertos.h"
#include "GawiButtons.h"
#include "LedMatrix.h"
#include "NetworkManager.h"
#include "utils.h"

enum class SystemState {
    BOOT, // 0
    NO_WIFI, // 1
    NO_DATA, // 2
    NO_API_RESPONSE, // 3
    DATA, // 4
    SETUP
};

class SystemManager {
    // Class instanses
    NetworkManager networkManager;
    Button* mainButton;
    ButtonManager buttonMng;
    LedMatrix matrix;

    // Variables
    SystemState systemState;
    SystemState prevSystemState;
    unsigned long now;
    unsigned long prevTime;
    uint8_t receiveNum = 0;
    QueuePacket receivedData;
    bool newData;

    // Tasks
    TaskHandle_t systemTaskHandle = nullptr;
    TaskHandle_t networkTaskHandle = nullptr;

    // Queues
    QueueHandle_t dataQueue = nullptr;

    // Methods
    void setSystemState(EventType event);
public:
    SystemManager();
    void init();
    void run();
    static void systemTask(void* pvParameters);
    static void networkTask(void* pvParameters);
};
