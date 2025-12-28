#pragma once
#include <Arduino.h>
#include "config.h"


enum class NetworkState {
    INIT,
    CONNECTING_STA,
    CONNECTED_STA,
    DISCONNECTED,
    ERROR
};

class NetworkManager {
    QueueHandle_t dataQueue = nullptr;
    NetworkState networkState;
    const unsigned long reconnectTiming = 5'000;
    const unsigned long apiTiming = 10'000;
    unsigned long prevReconnectAttempt;
    unsigned long prevApiFetch;
    uint8_t reconnectionAttempts;
    String apiId = "v1/sites/9143/departures?&forecast=360";
    String apiCombinedURL = API_URL + apiId;
public:
    NetworkManager();
    void init(QueueHandle_t queue);
    void run();
    void wifiConnect();
    void fetchApi();
    void parseJson(String apiPayload);
};