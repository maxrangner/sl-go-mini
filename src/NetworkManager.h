#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"

enum class NetworkState {
    INIT,
    CONNECTING_STA,
    CONNECTED_STA,
    DISCONNECTED,
    ERROR
};

struct Departure {
    char time[10];
    // char destination[25];
    // char direction[25];
    uint8_t directionCode;
    // uint8_t line;
    // char state[25];
    // char transportMode[25];
    // char expected[25];
};

struct Direction {
    Departure departures[NUM_DEPARTURES];
    uint8_t count = 0;
};

struct QueueMessage{
    Direction direction[2];
    NetworkState networkMangerState;
};

class NetworkManager {
    QueueHandle_t dataQueue = nullptr;
    NetworkState networkState;
    const unsigned long reconnectTiming = 5'000;
    const unsigned long apiTiming = 10'000;
    unsigned long prevReconnectAttempt;
    unsigned long prevApiFetch;
    bool hasNewData;
    uint8_t reconnectionAttempts;
    String apiId = "v1/sites/9143/departures?&forecast=360";
    String apiCombinedURL = API_URL + apiId;
    QueueMessage latestData;
public:
    NetworkManager();
    void init(QueueHandle_t queue);
    void run();
    void wifiConnect();
    void wifiWaitingForConnection();
    void fetchApi();
    void parseJson(JsonDocument doc);
    void updateFields(Direction& directionObject, JsonVariant source);
    bool sendToQueue();
};