#include "SystemManager.h"

SystemManager::SystemManager() {
    
}

void SystemManager::init() {
    Serial.println("SystemManager init()");
    _Button = _ButtonManager.addButton(BUTTON_PIN, true);
    Matrix.init();
}

void SystemManager::run() {
    _ButtonManager.updateAll();
    if (_Button->wasPushed()) Matrix.displayDeparture();
    if (_Button->wasHeld()) Matrix.changeColors();
    delay(50);
}
