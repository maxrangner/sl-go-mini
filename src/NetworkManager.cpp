#include "NetworkManager.h"
#include "WiFi.h"
#include <esp_wifi.h>
#include "HTTPClient.h"
#include "utils.h"
#include "credentials.h"
#include "networkDebug.h"

#include <esp_heap_caps.h>

void debugPrint();

NetworkManager::NetworkManager() {}

void NetworkManager::init(QueueHandle_t _dataQueue, QueueHandle_t _settingsQueue) {
    dataQueue = _dataQueue;
    settingsQueue = _settingsQueue;
    networkState = NetworkState::INIT;
    prevNetworkState = NetworkState::ERROR;
    reconnectionAttempts = 0;
    prevReconnectAttempt = 0;
    prevApiFetch = 0;
    hasNewData = false;
}

void NetworkManager::run() {
    switch (networkState) {
        case NetworkState::INIT:
            waitForSettingsPackage();
            wifiInit();
            break;

        case NetworkState::CONNECTING_STA:
            onStateChange(EventType::NO_WIFI);
            if (WiFi.status() == WL_CONNECTED) {
                Serial.println(""); Serial.print("Connected with IP: "); Serial.println(WiFi.localIP());
                networkState = NetworkState::CONNECTED_STA;
                reconnectionAttempts = 0;
                return;
            }
            wifiReconnect();
            break;

        case NetworkState::CONNECTED_STA:
            onStateChange(EventType::NO_DATA);
            if (WiFi.status() != WL_CONNECTED) networkState = NetworkState::CONNECTING_STA;
            else fetchApi();
            break;

        case NetworkState::ERROR:
            onStateChange(EventType::NO_API_RESPONSE);
            break;
    }
    // if (hasNewData) sendToQueue(); // VARIABLE NOW UNUSED
    // debugPrint();
}

void NetworkManager::onStateChange(EventType event) {
    if (networkState == prevNetworkState) {
        return;
    }
    const char* stateStr = "UNKNOWN";
    switch (networkState) {
        case NetworkState::INIT:            stateStr = "INIT"; break;
        case NetworkState::CONNECTING_STA:  stateStr = "CONNECTING_STA"; break;
        case NetworkState::CONNECTED_STA:   stateStr = "CONNECTED_STA"; break;
        case NetworkState::DISCONNECTED:    stateStr = "DISCONNECTED"; break;
        case NetworkState::ERROR:           stateStr = "ERROR"; break;
    }
    Serial.print("NetworkState: ");
    Serial.println(stateStr);

    eventUpdate(event);
}

void NetworkManager::wifiInit() {
    Serial.println("wifiInit()");

    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    WiFi.begin(settingsData.setSsid, settingsData.setPassword);
    esp_wifi_set_ps(WIFI_PS_NONE);
    Serial.print("Connecting to WiFi: "); Serial.println(SSID); 
    networkState = NetworkState::CONNECTING_STA;
    prevReconnectAttempt = millis();
}

void NetworkManager::wifiReconnect() {
    unsigned long now = millis();
    if (now - prevReconnectAttempt >= reconnectTiming) {
        reconnectionAttempts++;
        if (reconnectionAttempts % 6 != 0) {
            Serial.println("reconnect()");
            WiFi.reconnect();
            prevReconnectAttempt = now;
        } else {
            wifiInit();
        }
    }
}

void NetworkManager::fetchApi() {
    unsigned long now = millis();
    String apiPayload;
    if (now - prevApiFetch >= apiTiming) {
        Serial.println(""); Serial.println("Fetching Api!");
        
        HTTPClient http;
        http.useHTTP10(true);
        http.begin(apiCombinedURL);
        int httpResponse = http.GET();
        if (httpResponse > 0) {
            JsonDocument payload;
            DeserializationError errorGettingJson = deserializeJson(payload, http.getStream());
            if (!errorGettingJson) {
                parseJson(payload);
                eventUpdate(EventType::DATA);
                sendToQueue();
                // NetworkDebug::debugPrintQueueMessage(latestData);
            } else eventUpdate(EventType::NO_API_RESPONSE);
        } else eventUpdate(EventType::NO_API_RESPONSE);
        http.end();
        vTaskDelay(pdMS_TO_TICKS(150));
        prevApiFetch = now;
    }
}

void NetworkManager::parseJson(JsonDocument payload) {
    JsonArray departures = payload["departures"];
    resetDirectionsCounts();

    for (JsonVariant departure : departures) {
        uint8_t direction = static_cast<uint8_t>(departure["direction_code"]) - 1;
        if (departure["state"] != "CANCELLED" && departure["line"]["transport_mode"] == "METRO") {
            updateFields(latestData.direction[direction], departure);
        }
    }
}

void NetworkManager::updateFields(Direction& directionObject, JsonVariant source) {
    uint8_t& index = directionObject.count;
    Departure& departure = directionObject.departures[index];

    if (index < NUM_DEPARTURES) {
        if (strchr(source["display"], ':') == NULL) { // Departure is in format: "xx min"
            departure.displayTimeType = TimeDisplayType::MINUTES;
            departure.minutes = Utils::convertTextToMinutes(source["display"]);
        } else { // Departure is in format: "xx:xx"
            departure.displayTimeType = TimeDisplayType::CLOCK_TIME;
            Utils::writeCharArray(
                departure.clock_time,
                sizeof(departure.clock_time),
                source["display"]
            );
        }
        departure.directionCode = source["direction_code"];
        index++;
    }
}

bool NetworkManager::sendToQueue() {
    BaseType_t returnStatus;
    returnStatus = xQueueOverwrite(dataQueue, (void *)&latestData);
    hasNewData = false;
    if (returnStatus == pdTRUE) {
        // Serial.println("Packet successfully sent to dataQueue.");
        return true;
    } else {
        Serial.println("Error sending packet with dataQueue.");
        return false;
    }
}

void NetworkManager::waitForSettingsPackage() {
    while (1) {
        if (xQueueReceive(settingsQueue, (void *)&settingsData, pdMS_TO_TICKS(300)) == pdTRUE) {
            Serial.print("Settings received on core: "); Serial.println(xPortGetCoreID());
            break;
        }
    }
}

void NetworkManager::eventUpdate(EventType event) {
    latestData.type = event;
    hasNewData = true;
    prevNetworkState = networkState;
}

void NetworkManager::resetDirectionsCounts() {
    latestData.direction[0].count = 0;
    latestData.direction[1].count = 0;
}

TransportMode NetworkManager::parseTransportMode(const char* input) {
    if (strcmp(input, "METRO") == 0) return TransportMode::METRO;
    else if (strcmp(input, "TRAM") == 0) return TransportMode::TRAM;
    else if (strcmp(input, "TRAIN") == 0) return TransportMode::TRAIN;
    else if (strcmp(input, "BUS") == 0) return TransportMode::BUS;
    else if (strcmp(input, "SHIP") == 0) return TransportMode::SHIP;
    else if (strcmp(input, "FERRY") == 0) return TransportMode::FERRY;
    else if (strcmp(input, "TAXI") == 0) return TransportMode::TAXI;
    return TransportMode::UNKNOWN;
}