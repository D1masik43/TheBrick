#include "System/systemDrivers.h"

QueueHandle_t buttonEventQueue;

void handleButtons()
{
    if (!mcpAvailable) return;

    auto &mcp = SystemDrivers::Get().GetMCP();

    static uint16_t lastMcpState = 0xFFFF;

    uint16_t currentMcpState;
    if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        currentMcpState = mcp.readGPIOAB();
        xSemaphoreGive(i2cMutex);
    } else {
        return;
    }

    for (int i = 0; i < 10; i++)
    {
        bool lastState = bitRead(lastMcpState, i);
        bool currentState = bitRead(currentMcpState, i);

        if (lastState == 1 && currentState == 0)
        {
            vTaskDelay(pdMS_TO_TICKS(20));
            if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
                currentMcpState = mcp.readGPIOAB();
                xSemaphoreGive(i2cMutex);
            } else {
                break;
            }
            if (bitRead(currentMcpState, i) == 0)
            {
                xQueueSend(buttonEventQueue, &i, 0);
                bool held = true;
                while (held) {
                    vTaskDelay(pdMS_TO_TICKS(10));
                    if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
                        held = (bitRead(mcp.readGPIOAB(), i) == 0);
                        xSemaphoreGive(i2cMutex);
                    }
                }
            }
        }
    }

    lastMcpState = currentMcpState;
}

void buttonTask(void *pvParameters)
{
    auto &mcp = SystemDrivers::Get().GetMCP();
    if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        for (int i = 0; i < 10; i++) {
            mcp.pinMode(i, INPUT_PULLUP);
        }
        xSemaphoreGive(i2cMutex);
    }

    while (true) {
        handleButtons();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
