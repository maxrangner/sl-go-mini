#include "SystemManager.h"

SystemManager::SystemManager() {
    
}

void SystemManager::init() {
    Serial.println("SystemManager init()");

    _Button = _ButtonManager.addButton(BUTTON_PIN, true);
    Matrix.init();

    dataQueue = xQueueCreate(QUEUE_LEN, sizeof(num));
    stateQueue = xQueueCreate(QUEUE_LEN, sizeof(num));
    dataQueue = xQueueCreate(QUEUE_LEN, sizeof(num));

    if (dataQueue != nullptr) {
        Serial.println("Queue created!");
    }
    Serial.print("Initializing SystemManager on core: "); Serial.println(xPortGetCoreID()); 
    
    xTaskCreatePinnedToCore(     // SystemManager Task
      systemManagerTask,         // Function to implement the task
      "systemManagerTask",       // Name of the task
      4096,                      // Stack size in words
      this,                      // Task input parameter
      2,                         // Priority of the task
      &systemManagerTaskHandle,  // Task handle.
      0                          // Core where the task should run
    ); 
    
    xTaskCreatePinnedToCore(     // UI Task
      uiTask,
      "uiTask",
      4096,
      this, 
      1,
      &uiTaskHandle,
      0
    ); 

    xTaskCreatePinnedToCore( // Data Task
      dataTask,
      "dataTask",
      8192,
      this,
      0,
      &dataTaskHandle,
      1
    ); 
}

void SystemManager::systemManagerTask(void* pvParameters) {
    Serial.print("systemmanagerTask running on core ");
    Serial.println(xPortGetCoreID());

    SystemManager* self = static_cast<SystemManager*>(pvParameters);
    BaseType_t queueReturnStatus;

    while(1) {
        // CODE
        vTaskDelay(pdMS_TO_TICKS(1));
    } 
}

void SystemManager::uiTask(void* pvParameters) {
    vTaskDelay(200);
    Serial.print("uiTask running on core ");
    Serial.println(xPortGetCoreID());

    SystemManager* self = static_cast<SystemManager*>(pvParameters);
    BaseType_t queueReturnStatus;
    while(1) {
        self->_ButtonManager.updateAll();
        if (self->_Button->wasPushed()) {
            self->Matrix.displayDeparture();
            Serial.print("Button pushed on core: "); Serial.println(xPortGetCoreID());
        }
        if (self->_Button->wasHeld()) {
            self->Matrix.changeColors();
            Serial.print("Button held on core: "); Serial.println(xPortGetCoreID());
        }
        queueReturnStatus = xQueueReceive(self->dataQueue, (void *)&self->receiveNum, sizeof(num));
        if (queueReturnStatus == pdTRUE) {
            Serial.print("Data received on core: "); Serial.print(xPortGetCoreID());
            Serial.print(" : "); Serial.println(self->receiveNum);
        }
        vTaskDelay(1);
    }
}

void SystemManager::dataTask(void* pvParameters) {
    Serial.print("dataTask running on core ");
    Serial.println(xPortGetCoreID());

    SystemManager* self = static_cast<SystemManager*>(pvParameters);
    BaseType_t queueReturnStatus;

    while(1) {
        queueReturnStatus = xQueueSend(self->dataQueue, (void*)&self->num, sizeof(num));
        if (queueReturnStatus == pdTRUE) {
            Serial.print("Data sent from core: "); Serial.print(xPortGetCoreID());
            Serial.print(" : "); Serial.println(self->num);
            self->num++;
        } else {
            Serial.print("Failed to send data from core: "); Serial.println(xPortGetCoreID());
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    } 
}