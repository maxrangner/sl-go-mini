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

void NetworkManager::init(QueueHandle_t queue) {
    dataQueue = queue;
    networkState = NetworkState::INIT;
    prevNetworkState = networkState;
    reconnectionAttempts = 0;
    prevReconnectAttempt = 0;
    prevApiFetch = 0;
    hasNewData = false;
}

void NetworkManager::run() {
    switch (networkState) {
        case NetworkState::INIT:
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
    if (hasNewData) sendToQueue();
    // debugPrint();
}

void NetworkManager::onStateChange(EventType event) {
    if (networkState != prevNetworkState) eventUpdate(event);
}

void NetworkManager::wifiInit() {
    Serial.println("wifiInit()");

    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID, PASSWORD);
    esp_wifi_set_ps(WIFI_PS_NONE);
    Serial.print("Connecting to WiFi: "); Serial.print(SSID); 
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
        directionObject.departures[index++].directionCode = source["direction_code"];
    }
}

bool NetworkManager::sendToQueue() {
    BaseType_t returnStatus;
    returnStatus = xQueueOverwrite(dataQueue, (void *)&latestData);
    hasNewData = false;
    if (returnStatus == pdTRUE) {
        // Serial.println("Packet successfully sent to queue.");
        return true;
    } else {
        // Serial.println("Error sending packet to queue.");
        return false;
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

// WIFI DEBUG PRINT
const char* netStateStr(NetworkState s) {
    switch (s) {
        case NetworkState::INIT: return "INIT";
        case NetworkState::CONNECTING_STA: return "CONNECTING";
        case NetworkState::CONNECTED_STA: return "CONNECTED";
        case NetworkState::DISCONNECTED: return "DISCONNECTED";
        case NetworkState::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

const char* wifiStatusStr(wl_status_t s) {
    switch (s) {
        case WL_IDLE_STATUS: return "IDLE";
        case WL_NO_SSID_AVAIL: return "NO_SSID";
        case WL_CONNECTED: return "CONNECTED";
        case WL_CONNECT_FAILED: return "CONNECT_FAILED";
        case WL_CONNECTION_LOST: return "CONNECTION_LOST";
        case WL_DISCONNECTED: return "DISCONNECTED";
        default: return "UNKNOWN";
    }
}

void NetworkManager::debugPrint() {
    unsigned long now = millis();

    Serial.print("[NET] ");
    Serial.print(netStateStr(networkState));

    if (networkState != prevNetworkState) {
        Serial.print(" (ENTRY)");
    } else {
        Serial.print("        ");
    }

    Serial.print(" | wifi=");
    Serial.print(wifiStatusStr(WiFi.status()));
    Serial.print("(");
    Serial.print(WiFi.status());
    Serial.print(")");

    if (networkState == NetworkState::CONNECTING_STA) {
        Serial.print(" | t=");
        Serial.print((now - prevReconnectAttempt) / 1000.0, 1);
        Serial.print("/20s");
    }

    Serial.println();
}
