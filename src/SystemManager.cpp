#include "SystemManager.h"

SystemManager::SystemManager() {
    
}

void SystemManager::init() {
    Serial.println("SystemManager init()");
    _Button = _ButtonManager.addButton(BUTTON_PIN, true);
    Matrix.init();
    // Tasks
    Serial.print("Initializing on core: "); Serial.println(xPortGetCoreID());
    xTaskCreatePinnedToCore(
      uiTask, /* Function to implement the task */
      "uiTask", /* Name of the task */
      10000,  /* Stack size in words */
      this,  /* Task input parameter */
      0,  /* Priority of the task */
      &uiTaskHandle,  /* Task handle. */
      0 /* Core where the task should run */
    ); 
    xTaskCreatePinnedToCore(
      dataTask,
      "dataTask",
      10000,
      this,
      1,
      &dataTaskHandle,
      1
    ); 
}

void SystemManager::uiTask(void* pvParameters) {
    delay(200);
    Serial.print("uiTask running on core ");
    Serial.println(xPortGetCoreID());

    SystemManager* self = static_cast<SystemManager*>(pvParameters);

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
        vTaskDelay(1);
    }
}

void SystemManager::dataTask(void* pvParameters) {
    Serial.print("dataTask running on core ");
    Serial.println(xPortGetCoreID());

    SystemManager* self = static_cast<SystemManager*>(pvParameters);

    while(1) {
        Serial.print("Hi from core: "); Serial.println(xPortGetCoreID());
        vTaskDelay(pdMS_TO_TICKS(1000));
    } 
}