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
    unsigned long animationFrame;
    unsigned long lastFrameTime;
    const unsigned long frameRate = 50; // 50 = 20fps, 33 = 30fps, 20 = 50fps, 17 = 60fps

    // Tasks
    TaskHandle_t systemTaskHandle = nullptr;
    TaskHandle_t networkTaskHandle = nullptr;

    // Queues
    QueueHandle_t dataQueue = nullptr;

    // Methods
    void setSystemState(EventType event);
    void checkForNewPackage();
    void updateAnimationFrame();
public:
    SystemManager();
    void init();
    void run();
    static void systemTask(void* pvParameters);
    static void networkTask(void* pvParameters);
};
