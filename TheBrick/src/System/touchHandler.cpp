#include "System/systemDrivers.h"
#include "System/systemCommon.h"
#include "System/touchHandler.h"

TouchPoint lastPoints[2];
bool lastTouchValid[2] = { false, false };

QueueHandle_t touchEventQueue;

void handleTouch() {
    arduino::ft6336<SCREEN_WIDTH, SCREEN_HEIGHT>& touch = SystemDrivers::Get().GetTouch();

    if (!touch.update()) return;

    TouchPoint points[2] = {};
    bool currentTouchValid[2] = { false, false };

    uint16_t x, y;

    if (touch.xy(&x, &y)) {
        currentTouchValid[0] = true;
        points[0] = { (int)x, (int)y, lastTouchValid[0] ? SLIDE : NONE };
    } else if (lastTouchValid[0]) {
        points[0] = { lastPoints[0].x, lastPoints[0].y, TAP };
    }

    if (touch.touches() > 1 && touch.xy2(&x, &y)) {
        currentTouchValid[1] = true;
        points[1] = { (int)x, (int)y, lastTouchValid[1] ? SLIDE : NONE };
    } else if (lastTouchValid[1]) {
        points[1] = { lastPoints[1].x, lastPoints[1].y, TAP };
    }

    int validCount = 0;
    if (points[0].type != NONE) validCount++;
    if (points[1].type != NONE) validCount++;

    if (validCount > 0) {
        xQueueSend(touchEventQueue, &points, 0);  // Send full array (structs)
    }

    lastPoints[0] = points[0];
    lastPoints[1] = points[1];
    lastTouchValid[0] = currentTouchValid[0];
    lastTouchValid[1] = currentTouchValid[1];
}

void touchTask(void *pvParameters) {
    while (true) {
        handleTouch();
        vTaskDelay(pdMS_TO_TICKS(10));  // 100Hz update rate
    }
}
