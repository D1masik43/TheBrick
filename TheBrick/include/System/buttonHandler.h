#pragma once
#include <Arduino.h>

extern QueueHandle_t buttonEventQueue; // Queue used to store button events
extern bool lastStates[];              // Stores last known states of buttons

// RTOS task that runs in the background to monitor button states
void buttonTask(void *pvParameters);

// Called periodically to handle debounced button input and encoder changes
void handleButtons();
