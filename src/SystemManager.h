#pragma once
#include "freertos.h"
#include "GawiButtons.h"
#include "LedMatrix.h"
#include "NetworkManager.h"
#include "utils.h"
#include "apiDataTypes.h"
#include "queuePackets.h"

enum class SystemState {
    BOOT, // 0
    NO_WIFI, // 1
    NO_DATA, // 2
    NO_API_RESPONSE, // 3
    DATA, // 4
    SETUP,
    ERROR
};

class SystemManager {
    // Class instanses
    NetworkManager networkManager;
    Button* mainButton;
    ButtonManager buttonMng;
    LedMatrix matrix;

    // System variables
    bool settingsSent;
    SystemState systemState;
    SystemState prevSystemState;
    unsigned long now;
    unsigned long prevTime;
    uint8_t receiveNum = 0;
    bool newData;
    unsigned long animationFrame;
    unsigned long lastFrameTime;
    const unsigned long frameRate = 50; // 50 = 20fps, 33 = 30fps, 20 = 50fps, 17 = 60fps
    bool bootFinished;
    bool bootSettingsSent;
    
    // Tasks
    TaskHandle_t systemTaskHandle = nullptr;
    TaskHandle_t networkTaskHandle = nullptr;
    
    // Queues
    QueueHandle_t dataQueue = nullptr;
    QueueHandle_t settingsQueue = nullptr;
    QueuePacket receivedData;
    SettingsPacket settingsData;

    // Methods
    void setSystemState(EventType event);
    void checkForNewPackage();
    void updateAnimationFrame();
    void sendSettingsPackage();
    bool onStateChange();
public:
    SystemManager();
    void init();
    void run();
    static void systemTask(void* pvParameters);
    static void networkTask(void* pvParameters);
};
