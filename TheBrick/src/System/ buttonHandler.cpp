#include "System/systemDrivers.h"

QueueHandle_t buttonEventQueue;

void handleButtons()
{
    auto &mcp = SystemDrivers::Get().GetMCP();

    static uint16_t lastMcpState = 0xFFFF; // Assume all unpressed (HIGH)

    uint16_t currentMcpState = mcp.readGPIOAB(); // Read all 16 inputs (A0-A7, B0-B7)

    for (int i = 0; i < 10; i++) // A0–A7, B0, B1
    {
        bool lastState = bitRead(lastMcpState, i);
        bool currentState = bitRead(currentMcpState, i);

        if (lastState == 1 && currentState == 0) // Falling edge (pressed)
        {
            vTaskDelay(pdMS_TO_TICKS(20)); // debounce
            currentMcpState = mcp.readGPIOAB();
            if (bitRead(currentMcpState, i) == 0)
            {
                xQueueSend(buttonEventQueue, &i, 0); // Send button index 0–9
                // Wait for release (optional)
                while (bitRead(mcp.readGPIOAB(), i) == 0)
                {
                    vTaskDelay(pdMS_TO_TICKS(10));
                }
            }
        }
    }

    lastMcpState = currentMcpState;
}

void buttonTask(void *pvParameters)
{
    auto &mcp = SystemDrivers::Get().GetMCP();
    for (int i = 0; i < 10; i++) {
        mcp.pinMode(i, INPUT_PULLUP);  // A0-A7, B0, B1
    }

    while (true) {
        handleButtons();
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}