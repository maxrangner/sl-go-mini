#pragma once
#include "NetworkManager.h"

namespace NetworkDebug {
    inline static void debugPrintQueueMessage(const QueuePacket& data) {
        char buffer[512];          // justera vid behov
        size_t offset = 0;

        for (uint8_t dir = 0; dir < 2; dir++) {
            offset += snprintf(
                buffer + offset,
                sizeof(buffer) - offset,
                "Direction %u     Num entries: %u\n",
                dir,
                data.direction[dir].count
            );

            for (uint8_t i = 0; i < data.direction[dir].count; i++) {
                offset += snprintf(
                    buffer + offset,
                    sizeof(buffer) - offset,
                    "  [%u] time = %s     dir = %u\n",
                    i,
                    data.direction[dir].departures[i].minutes,
                    data.direction[dir].departures[i].directionCode
                );
            }
        }

        Serial.print(buffer);
    }
}

