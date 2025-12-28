#include "SystemManager.h"
#include "credentials.h"

SystemManager::SystemManager() {
    
}

void SystemManager::init() {
    Serial.println("SystemManager init()");
    
    Matrix.init();
    wifiInit();
    dataQueue = xQueueCreate(QUEUE_LEN, 4);
    mainButton = ButtonMng.addButton(BUTTON_PIN, true);
    NetworkMng.init(dataQueue);

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
      &NetworkMng,
      0,
      &networkTaskHandle,
      1
    ); 
}

void SystemManager::wifiInit() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID, PASSWORD);
    Serial.print("Connecting to wifi.");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        vTaskDelay(500);
    }
    Serial.println(WiFi.localIP());
}

void SystemManager::run() {
    ButtonMng.updateAll();
    if (mainButton->wasPushed()) {
        Matrix.displayDeparture();
        Serial.print("Button pushed on core: "); Serial.println(xPortGetCoreID());
    }
    if (mainButton->wasHeld()) {
        Matrix.changeColors();
        Serial.print("Button held on core: "); Serial.println(xPortGetCoreID());
    }
    BaseType_t queueReturnStatus = xQueueReceive(dataQueue, (void *)&receiveNum, 4);
    if (queueReturnStatus == pdTRUE) {
        Serial.print("Data received on core: "); Serial.print(xPortGetCoreID());
        Serial.print(" : "); Serial.println(receiveNum);
    }
}

void SystemManager::systemUiTask(void* pvParameters) {
    vTaskDelay(200);
    Serial.print("systemUiTask running on core ");
    Serial.println(xPortGetCoreID());

    SystemManager* self = static_cast<SystemManager*>(pvParameters);

    while(1) {
        self->run();
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