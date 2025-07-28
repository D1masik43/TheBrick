#pragma once
#include "System/systemStructs.h"
#include <Arduino.h>

extern QueueHandle_t touchEventQueue;

void handleTouch();
void touchTask(void *pvParameters);
