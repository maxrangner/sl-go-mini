#include "NetworkManager.h"
#include "WiFi.h"
#include "HTTPClient.h"
#include <ArduinoJson.h>
#include "credentials.h"

NetworkManager::NetworkManager() {

}

void NetworkManager::init(QueueHandle_t queue) {
    dataQueue = queue;
    networkState = NetworkState::INIT;
    reconnectionAttempts = 0;
    prevReconnectAttempt = 0;
    prevApiFetch = 0;
}

void NetworkManager::run() {
    switch (networkState) {
        case NetworkState::INIT:
            wifiConnect();
            break;
        case NetworkState::CONNECTING_STA:
            if (WiFi.status() != WL_CONNECTED) {
                Serial.print(".");
                vTaskDelay(500);
            } else if (WiFi.status() == WL_CONNECTED) {
                Serial.println(""); Serial.print("Connected with IP: "); Serial.print(WiFi.localIP());
                networkState = NetworkState::CONNECTED_STA;
            }
            break;
        case NetworkState::CONNECTED_STA:
            if (WiFi.status() != WL_CONNECTED) {
                networkState = NetworkState::DISCONNECTED;
            }
            fetchApi();
            break;
        case NetworkState::DISCONNECTED:
            wifiConnect();
            break;
        default:
            break;
    }
}

void NetworkManager::wifiConnect() {
    Serial.println("wifiConnect()");
    unsigned long now = millis();
    if (networkState == NetworkState::INIT || reconnectionAttempts > 5) {
        WiFi.disconnect();
        WiFi.mode(WIFI_STA);
        WiFi.begin(SSID, PASSWORD);
        Serial.print("Connecting to WiFi: "); Serial.print(SSID); 
        networkState = NetworkState::CONNECTING_STA;
        reconnectionAttempts = 0;
    } else if (now - prevReconnectAttempt >= reconnectTiming) {
        WiFi.reconnect();
        prevReconnectAttempt = now;
        reconnectionAttempts++;
    }
}

void NetworkManager::fetchApi() {
    Serial.println("fetchApi()");
    unsigned long now = millis();
    String apiPayload;
    if (now - prevApiFetch >= apiTiming) {
        Serial.println("Fetching Api!");
        HTTPClient http;
        http.begin(apiCombinedURL);
        uint8_t httpResponse = http.GET();
        if (httpResponse > 0) {
            Serial.print("HTTP Response code: "); Serial.println(httpResponse);
            apiPayload = http.getString();
            parseJson(apiPayload);
        }
        http.end();
        prevApiFetch = now;
    }
}

void NetworkManager::parseJson(String apiPayload) {
    JsonDocument payload;
    deserializeJson(payload, apiPayload);
    JsonArray departures = payload["departures"];
    for (JsonVariant item : departures) {
        serializeJsonPretty(item, Serial);
    }
}