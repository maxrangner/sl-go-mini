#include <Arduino.h>
#include "config.h"
#include "SystemManager.h"
/* To be included:
#include "WebServer.h"
#include "Preferences.h"
*/

SystemManager SysMng;

void setup() {
  Serial.begin(115200);
  delay(1500);
  Serial.println("BOOT!");
  Serial.printf("CPU Frequency: %d MHz\n", getCpuFrequencyMhz());
  Serial.printf("XTAL Frequency: %d MHz\n", getXtalFrequencyMhz());
  Serial.printf("APB Frequency: %d Hz\n", getApbFrequency());
  SysMng.init();
}

void loop() {
  // Everything is run through the systemTask and networkTask in SystemManager
}
