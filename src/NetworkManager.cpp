#include "NetworkManager.h"
#include "WiFi.h"
#include "HTTPClient.h"
#include "utils.h"
#include "credentials.h"
#include "networkDebug.h"

void debugPrint();

NetworkManager::NetworkManager() {}

void NetworkManager::init(QueueHandle_t queue) {
    dataQueue = queue;
    networkState = NetworkState::INIT;
    prevNetworkState = networkState;
    reconnectionAttempts = 0;
    timeReconnecting = 0;
    prevApiFetch = 0;
    hasNewData = false;
}

void NetworkManager::run() {
    switch (networkState) {
        case NetworkState::INIT:
            Serial.println("NetworkState::INIT");
            wifiInit();
            break;

        case NetworkState::CONNECTING_STA:
            if (networkState != prevNetworkState) {
                Serial.println("NetworkState::CONNECTING_STA");
                latestData.type = EventType::NO_WIFI;
                hasNewData = true;
            }
            wifiWaitingForConnection();
            break;

        case NetworkState::CONNECTED_STA:
            if (networkState != prevNetworkState) Serial.println("NetworkState::CONNECTED_STA");
            if (WiFi.status() != WL_CONNECTED) {
                networkState = NetworkState::DISCONNECTED;
            } else {
                fetchApi();
            }
            break;

        case NetworkState::DISCONNECTED:
            if (networkState != prevNetworkState) {
                Serial.println("NetworkState::DISCONNECTED");
                latestData.type = EventType::NO_WIFI;
                hasNewData = true;
            }
            if (reconnectionAttempts < 5) {
                WiFi.reconnect();
                networkState = NetworkState::CONNECTING_STA;
                timeReconnecting = millis();
                reconnectionAttempts++;
            } else {
                reconnectionAttempts = 0;
                wifiInit();
            }
            break;

        case NetworkState::ERROR:
        Serial.println("Error");
            if (networkState != prevNetworkState) {
            }
            break;

        default:
            break;
    }
    if (networkState != prevNetworkState) {
        prevNetworkState = networkState;
    } 
    if (hasNewData) sendToQueue();
    debugPrint();
}

void NetworkManager::wifiInit() {
    Serial.println("wifiInit()");

    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID, PASSWORD);
    Serial.print("Connecting to WiFi: "); Serial.print(SSID); 
    networkState = NetworkState::CONNECTING_STA;
    timeReconnecting = millis();
}

void NetworkManager::wifiWaitingForConnection() {
    unsigned long now = millis();
    if (now - timeReconnecting < 50'000) {
        if (WiFi.status() != WL_CONNECTED) {
            Serial.print(".");
            vTaskDelay(pdMS_TO_TICKS(500));
        } else if (WiFi.status() == WL_CONNECTED) {
            Serial.println(""); Serial.print("Connected with IP: "); Serial.println(WiFi.localIP());
            networkState = NetworkState::CONNECTED_STA;
        }
    } else networkState = NetworkState::ERROR;
}

void NetworkManager::fetchApi() {
    unsigned long now = millis();
    String apiPayload;
    if (now - prevApiFetch >= apiTiming) {
        Serial.println(""); Serial.println("Fetching Api!");

        HTTPClient http;
        http.useHTTP10(true);
        http.begin(apiCombinedURL);
        uint8_t httpResponse = http.GET();
        if (httpResponse > 0) {
            JsonDocument payload;
            DeserializationError error = deserializeJson(payload, http.getStream());
            if (!error) {
                parseJson(payload);
                latestData.type = EventType::DATA;
                hasNewData = true;
            } else {
                latestData.type = EventType::NO_DATA;
                hasNewData = true;
            }
        } else {
            latestData.type = EventType::NO_DATA;
            hasNewData = true;
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
    // Serial.println("Parsed json: ");
    // NetworkDebug::debugPrintQueueMessage(latestData);
}

void NetworkManager::updateFields(Direction& directionObject, JsonVariant source) {
    if (directionObject.count < NUM_DEPARTURES) {
        // Time to departure
        if (strchr(source["display"], ':') == NULL) {
            directionObject.departures[directionObject.count].displayTimeType = TimeDisplayType::MINUTES;
            char convertedMinutes[10];
            Utils::convertTexttoMinutes(convertedMinutes, sizeof(convertedMinutes), source["display"]);
            Utils::writeCharArray(
                directionObject.departures[directionObject.count].time,
                sizeof(directionObject.departures[directionObject.count].time),
                convertedMinutes
            );
        } else {
            directionObject.departures[directionObject.count].displayTimeType = TimeDisplayType::CLOCK_TIME;
            Utils::writeCharArray(
                directionObject.departures[directionObject.count].time,
                sizeof(directionObject.departures[directionObject.count].time),
                source["display"]
            );
        }
        // Direction code
        directionObject.departures[directionObject.count].directionCode = source["direction_code"];

        directionObject.count++;
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
        Serial.print((now - timeReconnecting) / 1000.0, 1);
        Serial.print("/20s");
    }

    Serial.println();
}
