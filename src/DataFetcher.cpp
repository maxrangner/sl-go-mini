#include "DataFetcher.h"

DataFetcher::DataFetcher() {

}

void DataFetcher::init(QueueHandle_t queue) {
    dataQueue = queue;
}

void DataFetcher::run() {
    
}