#include "System/SystemUI/StatusBar.h"
#include "System/systemDrivers.h"
#include <Arduino.h>

StatusBar& StatusBar::Get() {
    static StatusBar instance;
    return instance;
}

void StatusBar::Setup() {
    rtc = &SystemDrivers::Get().GetRTC();

    // Create FreeRTOS task for updating battery + SIM800
    xTaskCreatePinnedToCore(
        [](void* param){ ((StatusBar*)param)->statusTask(param); },
        "StatusTask",
        4096, 
        this,
        1, 
        nullptr,
        1 // core 1
    );
}

void StatusBar::statusTask(void* param) {
    StatusBar* self = (StatusBar*)param;
    HardwareSerial& sim800 = SystemDrivers::Get().GetSim800();

    for(;;) {
        // ---- Time ----
        DateTime now = self->rtc->now();
        char buffer[6];
        snprintf(buffer, sizeof(buffer), "%02d:%02d", now.hour(), now.minute());
        self->cachedTime = String(buffer);

        // ---- Battery ----
        self->cachedBattery = 75; // replace with actual read

        // ---- SIM800 ----
        sim800.println("AT+CSQ");
        delay(100);
        String response = "";
        while (sim800.available()) response += (char)sim800.read();
        int rssi = -1;
        if (response.indexOf("+CSQ:") != -1) {
            int start = response.indexOf(":") + 2;
            int end = response.indexOf(",", start);
            String rssiStr = response.substring(start, end);
            rssi = rssiStr.toInt();
        }
        if (rssi < 0 || rssi == 99) self->cachedBars = 0;
        else if (rssi < 10) self->cachedBars = 1;
        else if (rssi < 15) self->cachedBars = 2;
        else if (rssi < 20) self->cachedBars = 3;
        else self->cachedBars = 4;

        sim800.println("AT+CREG?");
        delay(100);
        response = "";
        while (sim800.available()) response += (char)sim800.read();
        self->cachedRegistered = (response.indexOf(",1") != -1 || response.indexOf(",5") != -1);

        vTaskDelay(pdMS_TO_TICKS(1000)); // update every 1 sec
    }
}


void StatusBar::Draw(TFT_eSprite& screenBuff, bool inMenu, uint16_t bg_color) {
    // Background
    if (!inMenu) {
        screenBuff.fillRect(0, 0, screenBuff.width(), 20, bg_color);
        screenBuff.setTextColor(TFT_WHITE, bg_color);
    } else {
        screenBuff.pushImage(0, 0, 240, 20, (const uint16_t*)wallpaperBlurred);
        screenBuff.setTextColor(TFT_WHITE);
    }

    screenBuff.setTextDatum(TL_DATUM);
    screenBuff.setTextSize(1);

    // Time
    screenBuff.drawString(cachedTime, 4, 4);

    // Battery
    String battStr = String(cachedBattery) + "%";
    int battX = screenBuff.width() - screenBuff.textWidth(battStr) - 4;
    screenBuff.drawString(battStr, battX, 4);

    // Signal bars / SIM800 registration
    if (cachedRegistered) {
        int barWidth = 3, barSpacing = 2, baseX = battX - (barWidth + barSpacing) * 4 - 8;
        for (int i = 0; i < 4; i++) {
            int h = (i + 1) * 3;
            uint16_t color = (i < cachedBars) ? TFT_WHITE : TFT_DARKGREY;
            screenBuff.fillRect(baseX + i * (barWidth + barSpacing), 16 - h, barWidth, h, color);
        }
    } else {
        // Draw red cross if not registered
        int size = 8;
        int x = battX - size - 4;
        int y = 4;
        screenBuff.drawLine(x, y, x + size, y + size, TFT_RED);
        screenBuff.drawLine(x + size, y, x, y + size, TFT_RED);
    }
}
