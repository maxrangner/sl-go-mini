#pragma once
#include <Arduino.h>

class DataFetcher {
    QueueHandle_t dataQueue = nullptr;
public:
    DataFetcher();
    void init(QueueHandle_t queue);
    void run();
};