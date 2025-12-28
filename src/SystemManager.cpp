#include "SystemManager.h"
#include "credentials.h"

SystemManager::SystemManager() {
    
}

void SystemManager::init() {
    Serial.println("SystemManager init()");
    
    matrix.init();
    dataQueue = xQueueCreate(QUEUE_LEN, 4);
    mainButton = buttonMng.addButton(BUTTON_PIN, true);
    dataFetcher.init(dataQueue);

    if (dataQueue != nullptr) {
        Serial.println("Queue created!");
    }
    Serial.print("Initializing SystemManager on core: "); Serial.println(xPortGetCoreID()); 
    
    xTaskCreatePinnedToCore(     // UI Task
      systemUiTask,              // Function to implement the task
      "systemUiTask",            // Name of the task
      4096,                      // Stack size in words
      this,                      // Task input parameter
      1,                         // Priority of the task
      &systemUiTaskHandle,       // Task handle.
      0                          // Core where the task should run
    ); 

    xTaskCreatePinnedToCore(     // Data Task
      dataFetcherTask,
      "dataFetcherTask",
      8192,
      &dataFetcher,
      0,
      &networkTaskHandle,
      1
    ); 
}

void SystemManager::run() {
    buttonMng.updateAll();
    switch (state) {
        case SystemState::BOOT:
            WiFi.mode(WIFI_STA);
            WiFi.begin(SSID, PASSWORD);
            state = SystemState::CONNECTING;
            break;
        case SystemState::CONNECTING:
            if (WiFi.status() == WL_CONNECTED) {
                Serial.println(WiFi.localIP());
                state = SystemState::CONNECTED;
            }
            break;
        case SystemState::CONNECTED:
            // Code
            break;
    }
}

void SystemManager::systemUiTask(void* pvParameters) {
    Serial.print("systemUiTask running on core ");
    Serial.println(xPortGetCoreID());

    SystemManager* systemManager = static_cast<SystemManager*>(pvParameters);

    while(1) {
        systemManager->run();
        vTaskDelay(10);
    }
}

void SystemManager::dataFetcherTask(void* pvParameters) {
    vTaskDelay(200);
    Serial.print("dataFetcherTask running on core ");
    Serial.println(xPortGetCoreID());

    DataFetcher* data = static_cast<DataFetcher*>(pvParameters);

    while(1) {
        data->run();
        vTaskDelay(pdMS_TO_TICKS(200));
    } 
}

// void SystemManager::run() {
//     buttonMng.updateAll();
//     if (mainButton->wasPushed()) {
//         matrix.displayDeparture();
//         Serial.print("Button pushed on core: "); Serial.println(xPortGetCoreID());
//     }
//     if (mainButton->wasHeld()) {
//         matrix.changeColors();
//         Serial.print("Button held on core: "); Serial.println(xPortGetCoreID());
//     }
//     BaseType_t queueReturnStatus = xQueueReceive(dataQueue, (void *)&receiveNum, 4);
//     if (queueReturnStatus == pdTRUE) {
//         Serial.print("Data received on core: "); Serial.print(xPortGetCoreID());
//         Serial.print(" : "); Serial.println(receiveNum);
//     }
// }