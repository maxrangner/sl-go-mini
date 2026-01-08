#include "SystemManager.h"
#include "networkDebug.h"

SystemManager::SystemManager() {
}

void SystemManager::init() {
    // Setup
    mainButton = buttonMng.addButton(BUTTON_PIN, true);
    systemState = SystemState::BOOT;
    prevSystemState = SystemState::ERROR;
    
    // Queues
    dataQueue = xQueueCreate(QUEUE_LEN, sizeof(QueuePacket));
    if (dataQueue != nullptr) Serial.println("Data queue created!");
    else Serial.println("Error creating queue.");
    settingsQueue = xQueueCreate(QUEUE_LEN, sizeof(SettingsPacket));
    if (settingsQueue != nullptr) Serial.println("Settings queue created!");
    else Serial.println("Error creating queue.");

    // Modules inits
    matrix.init();
    networkManager.init(dataQueue, settingsQueue);
    
    // Variables
    settingsSent = false;
    now = millis();
    prevTime = 0;
    newData = false;
    animationFrame = 0;
    lastFrameTime = 0;
    bootFinished = false;
    bootSettingsSent = false;

    // Settings
    settingsData.setTransportMode = TransportMode::METRO;
    settingsData.setDirectionCode = 1;
    strncpy(settingsData.setSsid, "MaxGuest", sizeof(settingsData.setSsid));
    strncpy(settingsData.setPassword, "yourewelcome", sizeof(settingsData.setPassword));
    
    // Create tasks
    xTaskCreatePinnedToCore(     // UI Task
      systemTask,                // Function to implement the task
      "systemUiTask",            // Name of the task
      8192,                      // Stack size in words
      this,                      // Task input parameter
      2,                         // Priority of the task
      &systemTaskHandle,         // Task handle.
      0                          // Core where the task should run
    ); 

    xTaskCreatePinnedToCore(     // Data Task
      networkTask,
      "networkTask",
      8192,
      &networkManager,
      1,
      &networkTaskHandle,
      1
    ); 
}

void SystemManager::run() {
    buttonMng.updateAll();
    if (bootFinished) checkForNewPackage();

    switch (systemState) {
        case SystemState::BOOT:
            if (onStateChange()) {
                Serial.println("SystemState::BOOT");
                sendSettingsPackage();
                matrix.displayIcon(0);
            }
            if (animationFrame > 100) {
                    Serial.println("*** Boot complete ***");
                    bootFinished = true;
            }
            break;
        case SystemState::NO_WIFI:
            if (onStateChange()) Serial.println("SystemState::NO_WIFI");
            matrix.connectionAnimation(animationFrame);
            break;
        case SystemState::NO_DATA:
            if (onStateChange()) Serial.println("SystemState::NO_DATA");
            matrix.connectionAnimation(animationFrame);
            break;
        case SystemState::DATA:
            // Bygg bort event drivet och ersätt med polling.
            if (onStateChange()) Serial.println("SystemState::DATA");
            if (newData) {
                uint8_t direction = settingsData.setDirectionCode - 1;

                if (receivedData.direction[direction].count > 0 && receivedData.direction[direction].departures[0].displayTimeType == TimeDisplayType::MINUTES) {
                    // Serial.println("TimeDisplayType::MINUTES");
                    matrix.displayDeparture(receivedData.direction[direction].departures[0].minutes); 
                    Serial.print("Next departure: "); Serial.print(receivedData.direction[direction].departures[0].minutes); Serial.println(" min");
                } else if (receivedData.direction[direction].count > 0 && receivedData.direction[direction].departures[0].displayTimeType == TimeDisplayType::CLOCK_TIME) {
                    // Serial.println("TimeDisplayType::CLOCK_TIME");
                    matrix.displayDeparture(receivedData.direction[settingsData.setDirectionCode + 1].departures[0].minutes); 
                    matrix.sleepAnimation(animationFrame);
                    Serial.print("Next departure: "); Serial.println(receivedData.direction[direction].departures[0].clock_time);
                }
                newData = false;
                prevTime = millis();
            }
            break;
        case SystemState::NO_API_RESPONSE:
            if (onStateChange()) Serial.println("SystemState::NO_API_RESPONSE");
            matrix.clear();
            break;
        case SystemState::SETUP:
            break;
    }
    if (mainButton->wasPushed()) {
        Serial.print("\nsystemState: "); Serial.println(static_cast<int>(systemState));
    }
    if (systemState != prevSystemState) {
        prevSystemState = systemState;
    }
    updateAnimationFrame();
}

void SystemManager::systemTask(void* pvParameters) {
    Serial.print("systemTask running on core ");
    Serial.println(xPortGetCoreID());

    SystemManager* systemManager = static_cast<SystemManager*>(pvParameters);

    while(1) {
        systemManager->run();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void SystemManager::networkTask(void* pvParameters) {
    vTaskDelay(pdMS_TO_TICKS(200));
    Serial.print("networkTask running on core ");
    Serial.println(xPortGetCoreID());

    NetworkManager* network = static_cast<NetworkManager*>(pvParameters);

    while(1) {
        network->run();
        vTaskDelay(pdMS_TO_TICKS(200));
    } 
}

void SystemManager::setSystemState(EventType event) {
    switch (event) {
        case EventType::NO_WIFI:
            systemState = SystemState::NO_WIFI;
            break;
        case EventType::NO_DATA:
            systemState = SystemState::NO_DATA;
            break;
        case EventType::DATA:
            systemState = SystemState::DATA;
            break;
        case EventType::NO_API_RESPONSE:
            systemState = SystemState::NO_API_RESPONSE;
            break;
    }
}

bool SystemManager::onStateChange() {
    if (systemState != prevSystemState) {
        Serial.print("SystemState: "); Serial.println(static_cast<int>(systemState));
        return true;
    }
    return false;
}

void SystemManager::checkForNewPackage() {
    if (xQueueReceive(dataQueue, (void *)&receivedData, 0) == pdTRUE) {
        Serial.println("Packet received!");
        setSystemState(receivedData.type);
        if (receivedData.type == EventType::DATA) {
            newData = true;
        } 
    }
}

void SystemManager::sendSettingsPackage() {
    BaseType_t returnStatus;
    xQueueOverwrite(settingsQueue, (void *)&settingsData);
}

void SystemManager::updateAnimationFrame() {
    unsigned long now = millis();
    if (now - lastFrameTime >= frameRate) {
        animationFrame++;
        lastFrameTime = now;
        // Serial.println(animationFrame);
    }
}