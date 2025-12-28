#pragma once
#include <Arduino.h>

class NetworkManager {
    QueueHandle_t dataQueue = nullptr;
public:
    NetworkManager();
    void init(QueueHandle_t queue);
    void loop();
};