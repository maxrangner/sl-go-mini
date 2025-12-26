#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "WiFi.h"
#include "WebServer.h"
#include "Preferences.h"
#include "HTTPClient.h"
#include "ArduinoJson.h"
#include "GawiButtons.h"
#include "config.h"

void setup() {
  Serial.begin(115200);
  delay(1500);
  Serial.println("BOOT!");
}

void loop() {

}