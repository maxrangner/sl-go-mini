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

enum class TimeDisplayType {
    MINUTES,
    CLOCK_TIME
};

enum class EventType {
    NO_WIFI,
    NO_DATA,
    NO_API_RESPONSE,
    DATA
};

struct Departure {
    TimeDisplayType displayTimeType;
    uint8_t minutes;
    char clock_time[10];
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

struct QueuePacket{
    EventType type;
    Direction direction[2];
};

class NetworkManager {
    QueueHandle_t dataQueue = nullptr;
    NetworkState networkState;
    NetworkState prevNetworkState;
    const unsigned long reconnectTiming = 10'000;
    const unsigned long apiTiming = 10'000;
    unsigned long timeReconnecting;
    unsigned long prevApiFetch;
    bool hasNewData;
    uint8_t reconnectionAttempts;
    String apiId = "v1/sites/9143/departures?&forecast=360";
    String apiCombinedURL = API_URL + apiId;
    QueuePacket latestData;
    void debugPrint();
    void wifiInit();
    void wifiWaitingForConnection();
    void fetchApi();
    void parseJson(JsonDocument doc);
    void updateFields(Direction& directionObject, JsonVariant source);
    bool sendToQueue();
    void eventUpdate(EventType event);
public:
    NetworkManager();
    void init(QueueHandle_t queue);
    void run();
};
