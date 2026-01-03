#include "SystemManager.h"
#include "networkDebug.h"

SystemManager::SystemManager() {
}

void SystemManager::init() {
    matrix.init();
    mainButton = buttonMng.addButton(BUTTON_PIN, true);
    dataQueue = xQueueCreate(QUEUE_LEN, sizeof(QueuePacket));
    if (dataQueue != nullptr) {
        Serial.println("Queue created!");
    }
    networkManager.init(dataQueue);
    now = millis();
    prevTime = 0;
    systemState = SystemState::BOOT;
    prevSystemState = systemState;
    
    // Create tasks
    xTaskCreatePinnedToCore(     // UI Task
      systemTask,                // Function to implement the task
      "systemUiTask",            // Name of the task
      4096,                      // Stack size in words
      this,                      // Task input parameter
      1,                         // Priority of the task
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
    if (xQueueReceive(dataQueue, (void *)&receivedData, sizeof(QueuePacket)) == true) {
        // Serial.print("Data received on core: "); Serial.println(xPortGetCoreID());
        setSystemState(receivedData.type);
        if (receivedData.type == EventType::DATA) {
            if (receivedData.direction[0].count > 0) {
                Serial.print("Next departure: "); Serial.print(receivedData.direction[0].departures[0].time); Serial.println(" min");
            }
        }
        // Serial.println(" : "); NetworkDebug::debugPrintQueueMessage(receivedData);
    }
    switch (systemState) {
        case SystemState::BOOT:
            Serial.println("SystemState::BOOT");
            systemState = SystemState::NO_WIFI;
            matrix.clearDisplay();
            break;
        case SystemState::NO_WIFI:
            if (systemState != prevSystemState) {
                Serial.println("SystemState::NO_WIFI");
            }
            matrix.clearDisplay();
            break;
        case SystemState::NO_DATA:
            if (systemState != prevSystemState) {
                Serial.println("SystemState::NO_DATA");
            }
            matrix.clearDisplay();
            break;
        case SystemState::DATA:
            if (systemState != prevSystemState) {
                Serial.println("SystemState::DATA");
            }
            matrix.displayDeparture(receivedData.direction[0].departures[0].time);
            break;
        case SystemState::NO_API_RESPONSE:
            if (systemState != prevSystemState) {
                Serial.println("SystemState::NO_API_RESPONSE");
            }
            matrix.clearDisplay();
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
    vTaskDelay(200);
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