#include "System/touchHandler.h"

TouchPoint lastPoints[2];
bool lastTouchValid[2] = { false, false };
QueueHandle_t touchEventQueue;
bool inSlide[2] = { false, false };

bool movedEnough(int x1, int y1, int x2, int y2, int threshold = 0) {
    return (std::abs(x1 - x2) > threshold) || (std::abs(y1 - y2) > threshold);
}

void handleTouch() {
    arduino::ft6336<SCREEN_WIDTH, SCREEN_HEIGHT>& touch = SystemDrivers::Get().GetTouch();

    if (!touch.update()) return;

    TouchPoint points[2] = {};
    bool currentTouchValid[2] = { false, false };

    uint16_t x, y;

    // Touch 0
    if (touch.xy(&x, &y)) {
    currentTouchValid[0] = true;
    TouchType type = NONE;

    if (lastTouchValid[0]) {
        if (inSlide[0]) {
            if (movedEnough(x, y, lastPoints[0].x, lastPoints[0].y)) {
                type = SLIDE;
            }
        } else {
            if (movedEnough(x, y, lastPoints[0].x, lastPoints[0].y)) {
                type = SLIDE_BEGIN;
                inSlide[0] = true;
            }
        }
    }

    points[0] = { (int)x, (int)y, type };
    } else if (lastTouchValid[0]) {
        TouchType type = inSlide[0] ? SLIDE_END : TAP;
        points[0] = { lastPoints[0].x, lastPoints[0].y, type };
        inSlide[0] = false;
        lastTouchValid[0] = false;
    }


    // Touch 1
    if (touch.touches() > 1 && touch.xy2(&x, &y)) {
        currentTouchValid[1] = true;

        TouchType type = NONE;
        if (lastTouchValid[1]) {
            if (lastPoints[1].type == SLIDE) {
                if (movedEnough(x, y, lastPoints[1].x, lastPoints[1].y)) {
                    type = SLIDE;
                } else {
                    type = NONE;
                }
            } else {
                if (movedEnough(x, y, lastPoints[1].x, lastPoints[1].y)) {
                    type = SLIDE;
                } else {
                    type = NONE;
                }
            }
        } else {
            type = NONE;
        }

        points[1] = { (int)x, (int)y, type };
    } else if (lastTouchValid[1]) {
        points[1] = { lastPoints[1].x, lastPoints[1].y, TAP };
        lastTouchValid[1] = false;
    }

    int validCount = 0;
    if (points[0].type != NONE) validCount++;
    if (points[1].type != NONE) validCount++;

    if (validCount > 0) {
        xQueueSend(touchEventQueue, &points, 0);
    }

    lastPoints[0] = points[0];
    lastPoints[1] = points[1];
    lastTouchValid[0] = currentTouchValid[0];
    lastTouchValid[1] = currentTouchValid[1];
}

void touchTask(void *pvParameters) {
    int hz = 30;
    int tickMs = 1000 / hz;
    while (true) {
        handleTouch();
        vTaskDelay(pdMS_TO_TICKS(tickMs));
    }
}
