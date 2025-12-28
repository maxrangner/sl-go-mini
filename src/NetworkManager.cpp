#include "NetworkManager.h"

NetworkManager::NetworkManager() {

}

void NetworkManager::init(QueueHandle_t queue) {
    dataQueue = queue;
}

void NetworkManager::loop() {
    
}