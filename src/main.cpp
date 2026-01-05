#include <Arduino.h>
#include "config.h"
#include "SystemManager.h"
#include "esp_pm.h"
/* To be included:
#include "WebServer.h"
#include "Preferences.h"
*/

SystemManager SysMng;

void setup() {
  Serial.begin(115200);
  delay(1500);
  /* vvv Needed? vvv */
  esp_pm_config_esp32s3_t pm_config = {
    .max_freq_mhz = 240,
    .min_freq_mhz = 240,
    .light_sleep_enable = false
  };
  esp_pm_configure(&pm_config);
  /* ^^^ Needed? ^^^ */
  Serial.println("BOOT!");
  SysMng.init();
}

void loop() {
  // Everything is run through the systemTask and networkTask in SystemManager
}
