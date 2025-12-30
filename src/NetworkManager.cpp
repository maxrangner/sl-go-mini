#include "NetworkManager.h"
#include "WiFi.h"
#include "HTTPClient.h"
#include "utils.h"
#include "credentials.h"

NetworkManager::NetworkManager() {}

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
            wifiWaitingForConnection();
            break;
        case NetworkState::CONNECTED_STA:
            if (WiFi.status() != WL_CONNECTED) {
                networkState = NetworkState::DISCONNECTED;
            } else {
                fetchApi();
            }
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

void NetworkManager::wifiWaitingForConnection() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        vTaskDelay(500);
    } else if (WiFi.status() == WL_CONNECTED) {
        Serial.println(""); Serial.print("Connected with IP: "); Serial.print(WiFi.localIP());
        networkState = NetworkState::CONNECTED_STA;
    }
}

void NetworkManager::fetchApi() {
    Serial.println("fetchApi()");
    unsigned long now = millis();
    String apiPayload;
    if (now - prevApiFetch >= apiTiming) {
        Serial.println("Fetching Api!");

        HTTPClient http;
        http.useHTTP10(true);
        http.begin(apiCombinedURL);
        uint8_t httpResponse = http.GET();
        if (httpResponse > 0) {
            Serial.print("HTTP Response code: "); Serial.println(httpResponse);

            JsonDocument payload;
            DeserializationError error = deserializeJson(payload, http.getStream());
            if (!error) parseJson(payload);
        }
        http.end();
        prevApiFetch = now;
    }
}

void NetworkManager::parseJson(JsonDocument payload) {
    JsonArray departures = payload["departures"];
    Direction& direction0 = latestData.direction[0];
    Direction& direction1 = latestData.direction[1];
    direction0.count = 0;
    direction1.count = 0;

    for (JsonVariant departure : departures) {
        if (departure["direction_code"] == 1
            && departure["state"] != "CANCELLED"
            && departure["line"]["transport_mode"] == "METRO") {
            updateFields(direction0, departure);
        }
        if (departure["direction_code"] == 2
            && departure["state"] != "CANCELLED"
            && departure["line"]["transport_mode"] == "METRO") {
            updateFields(direction1, departure);
        }
    }
    debugPrintQueueMessage(latestData);
}

void NetworkManager::updateFields(Direction& directionObject, JsonVariant source) {
    if (directionObject.count < NUM_DEPARTURES) {
        Utils::writeCharArray( // Display (time)
            directionObject.departures[directionObject.count].time,
            sizeof(directionObject.departures[directionObject.count].time),
            source["display"]
        );
        directionObject.departures[directionObject.count].directionCode = source["direction_code"];
        directionObject.count++;
    }
}

static void debugPrintQueueMessage(const QueueMessage& data) {
    for (uint8_t dir = 0; dir < 2; dir++) {
        Serial.print("Direction ");
        Serial.print(dir);
        Serial.print("     Num entries: ");
        Serial.println(data.direction[dir].count);

        for (uint8_t i = 0; i < data.direction[dir].count; i++) {
            Serial.print("  [");
            Serial.print(i);
            Serial.print("] time = ");
            Serial.print(data.direction[dir].departures[i].time);
            Serial.print("     dir = ");
            Serial.println(data.direction[dir].departures[i].directionCode);
        }
    }
}