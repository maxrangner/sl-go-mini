#include "SystemManager.h"
#include "networkDebug.h"

SystemManager::SystemManager() {
}

void SystemManager::init() {
    matrix.init();
    mainButton = buttonMng.addButton(BUTTON_PIN, true);
    dataQueue = xQueueCreate(QUEUE_LEN, sizeof(QueueMessage));
    if (dataQueue != nullptr) {
        Serial.println("Queue created!");
    }
    networkManager.init(dataQueue);
    now = millis();
    prevTime = 0;
    
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
    switch (state) {
        case SystemState::BOOT:
            if (mainButton->wasPushed()) {
                Serial.print("Button pushed on core: "); Serial.println(xPortGetCoreID());
            }
            if (xQueueReceive(dataQueue, (void *)&receivedData, sizeof(QueueMessage)) == true) {
                Serial.print("Data received on core: "); Serial.print(xPortGetCoreID());
                Serial.println(" : "); NetworkDebug::debugPrintQueueMessage(receivedData);
            }
            break;
        case SystemState::CONNECTING:
            break;
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
