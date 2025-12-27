#pragma once
#include <Arduino.h>

constexpr const uint8_t LED_PIN = 14;
constexpr const uint8_t PIXELS_NUM = 64;
constexpr const uint8_t BUTTON_PIN = 0;
constexpr const uint8_t QUEUE_LEN = 1;
constexpr const uint8_t QUEUE_ITEM_SIZE = 4;
constexpr const char* API_URL = "https://transport.integration.sl.se/v1/sites/9143/departures?&forecast=360";