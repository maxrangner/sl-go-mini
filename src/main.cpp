#include <Arduino.h>
#include <ArduinoJson.h>
#include "WiFi.h"
#include "WebServer.h"
#include "Preferences.h"
#include "HTTPClient.h"
#include "config.h"
#include "SystemManager.h"

SystemManager SysMng;

void setup() {
  Serial.begin(115200);
  delay(1500);
  Serial.println("BOOT!");
  SysMng.init();
}

void loop() {
  SysMng.run();
}

// Få klick att flytta tänd pixel ett steg. Håll in för att byta färg.