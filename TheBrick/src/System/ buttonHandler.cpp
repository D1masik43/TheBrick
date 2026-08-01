#include "System/systemDrivers.h"

QueueHandle_t buttonEventQueue;

static const int buttonPins[] = {
    BUTTON_BACK,    // 0
    BUTTON_HOME,    // 1
    BUTTON_IN,      // 2
    BUTTON_KEY1,    // 3
    BUTTON_LEFT,    // 9
    BUTTON_RIGHT,   // 10
    BUTTON_DOWN,    // 11
};
static const int numButtons = sizeof(buttonPins) / sizeof(buttonPins[0]);

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

    for (int b = 0; b < numButtons; b++)
    {
        int i = buttonPins[b];
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
                int timeout = 200;
                while (held && timeout-- > 0) {
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
        for (int i = 0; i < 16; i++) {
            if (i == AMP_SD_PIN || i == LED_PIN) continue;
            mcp.pinMode(i, INPUT_PULLUP);
        }
        xSemaphoreGive(i2cMutex);
    }

    while (true) {
        handleButtons();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
