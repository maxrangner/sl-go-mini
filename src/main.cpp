#include <Arduino.h>
#include "config.h"
#include "SystemManager.h"
/* To be included:
#include "WiFi.h"
#include <ArduinoJson.h>
#include "WebServer.h"
#include "Preferences.h"
#include "HTTPClient.h"
*/

SystemManager SysMng;

void setup() {
  Serial.begin(115200);
  delay(1500);
  Serial.println("BOOT!");
  SysMng.init();
}

void loop() {
}
