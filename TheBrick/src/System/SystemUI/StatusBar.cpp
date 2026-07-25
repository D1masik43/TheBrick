#include "System/SystemUI/StatusBar.h"
#include "System/systemDrivers.h"

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

static String sim800Command(HardwareSerial& sim800, const char* cmd, int waitMs = 200) {
    while (sim800.available()) sim800.read();
    sim800.println(cmd);
    delay(waitMs);
    String response = "";
    while (sim800.available()) response += (char)sim800.read();
    return response;
}

void StatusBar::statusTask(void* param) {
    StatusBar* self = (StatusBar*)param;
    HardwareSerial& sim800 = SystemDrivers::Get().GetSim800();

    // Kick SIM800 awake
    sim800Command(sim800, "AT", 100);
    sim800Command(sim800, "AT", 100);
    sim800Command(sim800, "ATE0", 100);
    sim800Command(sim800, "AT+CFUN=1", 300);

    int pollDelay = 200;

    for(;;) {
        // ---- Time ----
        if (rtcAvailable && xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            DateTime now = self->rtc->now();
            xSemaphoreGive(i2cMutex);
            char buffer[6];
            snprintf(buffer, sizeof(buffer), "%02d:%02d", now.hour(), now.minute());
            self->cachedTime = String(buffer);
        }

        // ---- Battery ----
        if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            auto &ina = SystemDrivers::Get().GetINA219();
            float v = ina.getBusVoltage_V();
            float mA = ina.getCurrent_mA();
            xSemaphoreGive(i2cMutex);
            int pct = (int)(v / 4.2 * 100);
            self->cachedBattery = pct;
            self->cachedCharging = (mA > 0);
        }

        // ---- SIM800 ----
        String response = sim800Command(sim800, "AT+CSQ");
        int rssi = -1;
        if (response.indexOf("+CSQ:") != -1) {
            int start = response.indexOf(":") + 2;
            int end = response.indexOf(",", start);
            rssi = response.substring(start, end).toInt();
        }
        if (rssi < 0 || rssi == 99) self->cachedBars = 0;
        else if (rssi < 10) self->cachedBars = 1;
        else if (rssi < 15) self->cachedBars = 2;
        else if (rssi < 20) self->cachedBars = 3;
        else self->cachedBars = 4;

        response = sim800Command(sim800, "AT+CREG?");
        self->cachedRegistered = (response.indexOf(",1") != -1 || response.indexOf(",5") != -1);

        if (self->cachedRegistered && pollDelay < 1000) pollDelay = 1000;

        vTaskDelay(pdMS_TO_TICKS(pollDelay));
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
    if (cachedCharging) battStr += "+";
    int battX = screenBuff.width() - screenBuff.textWidth(battStr) - 4;
    screenBuff.setTextColor(cachedCharging ? TFT_GREEN : TFT_WHITE);
    screenBuff.drawString(battStr, battX, 4);
    screenBuff.setTextColor(TFT_WHITE);

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
