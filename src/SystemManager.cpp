#include "SystemManager.h"
#include "credentials.h"

SystemManager::SystemManager() {
    
}

void SystemManager::init() {
    Serial.println("SystemManager init()");

    _Button = _ButtonManager.addButton(BUTTON_PIN, true);
    _Matrix.init();
    dataQueue = xQueueCreate(QUEUE_LEN, 4);
    _NetworkMng.init(dataQueue);

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
      networkTask,
      "dataTask",
      8192,
      &_NetworkMng,
      0,
      &networkTaskHandle,
      1
    ); 
}

void SystemManager::wifiInit() {
    WiFi.mode(WIFI_STA);
    WiFi.begin();
    Serial.print("Connecting to wifi.");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        vTaskDelay(500);
    }
    Serial.println(WiFi.localIP());
}

void SystemManager::systemUiTask(void* pvParameters) {
    vTaskDelay(200);
    Serial.print("systemUiTask running on core ");
    Serial.println(xPortGetCoreID());

    SystemManager* self = static_cast<SystemManager*>(pvParameters);
    BaseType_t queueReturnStatus;

    while(1) {
        self->_ButtonManager.updateAll();
        if (self->_Button->wasPushed()) {
            self->_Matrix.displayDeparture();
            Serial.print("Button pushed on core: "); Serial.println(xPortGetCoreID());
        }
        if (self->_Button->wasHeld()) {
            self->_Matrix.changeColors();
            Serial.print("Button held on core: "); Serial.println(xPortGetCoreID());
        }
        queueReturnStatus = xQueueReceive(self->dataQueue, (void *)&self->receiveNum, 4);
        if (queueReturnStatus == pdTRUE) {
            Serial.print("Data received on core: "); Serial.print(xPortGetCoreID());
            Serial.print(" : "); Serial.println(self->receiveNum);
        }
        vTaskDelay(1);
    }
}

void SystemManager::networkTask(void* pvParameters) {
    Serial.print("networkTask running on core ");
    Serial.println(xPortGetCoreID());

    NetworkManager* net = static_cast<NetworkManager*>(pvParameters);

    BaseType_t queueReturnStatus;

    while(1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    } 
}