#include <Arduino.h>
#include "config.h"
#include "SystemManager.h"
/* To be included:
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
  // Everything is run through the systemUiTask and dataFetcherTask in SystemManager
}
