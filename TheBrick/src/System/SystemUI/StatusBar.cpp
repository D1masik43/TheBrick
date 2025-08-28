#include "System/SystemUI/StatusBar.h"
#include "System/systemDrivers.h"
#include <Arduino.h>

StatusBar& StatusBar::Get() {
    static StatusBar instance;
    return instance;
}

void StatusBar::Setup() {
    rtc = &SystemDrivers::Get().GetRTC();
}

String StatusBar::getTimeString() {
    unsigned long nowMs = millis();
    if (nowMs - lastTimeUpdate >= 1000) {
        lastTimeUpdate = nowMs;
            DateTime now = rtc->now();
            char buffer[6];
            snprintf(buffer, sizeof(buffer), "%02d:%02d", now.hour(), now.minute());
            cachedTime = String(buffer);
    }
    return cachedTime;
}

int StatusBar::getBatteryPercent() {
    return -1;
}

int StatusBar::getSim800SignalStrength() {
    HardwareSerial &sim800 = SystemDrivers::Get().GetSim800();
    sim800.println("AT+CSQ");
    delay(100);

    String response = "";
    while (sim800.available()) {
        response += (char)sim800.read();
    }

    int rssi = -1;
    if (response.indexOf("+CSQ:") != -1) {
        int start = response.indexOf(":") + 2;
        int end = response.indexOf(",", start);
        String rssiStr = response.substring(start, end);
        rssi = rssiStr.toInt(); // 0-31, 99 = unknown
    }

    // Map RSSI (0-31) to 0-4 bars
    if (rssi == 99 || rssi < 0) return 0;
    if (rssi < 10) return 1;  // weak
    if (rssi < 15) return 2;  // fair
    if (rssi < 20) return 3;  // good
    return 4;                 // excellent
}

bool StatusBar::isSim800Registered() {
    HardwareSerial &sim800 = SystemDrivers::Get().GetSim800();
    sim800.println("AT+CREG?");
    delay(100);

    String response = "";
    while (sim800.available()) {
        response += (char)sim800.read();
    }

    // +CREG: 0,1 or 0,5 means registered
    return (response.indexOf(",1") != -1 || response.indexOf(",5") != -1);
}


void StatusBar::Draw(TFT_eSprite& screenBuff, bool inMenu, uint16_t bg_color) {
    if (!inMenu) {
        screenBuff.fillRect(0, 0, screenBuff.width(), 20, bg_color);
    } else {
        screenBuff.pushImage(0, 0, 240, 20, (const uint16_t*)wallpaperBlurred);
    }

    if(bg_color != NO_BG_COLOR)
    {
        screenBuff.setTextColor(TFT_WHITE, bg_color);
    }else
    {
        screenBuff.setTextColor(TFT_WHITE);
    }

    screenBuff.setTextDatum(TL_DATUM);
    screenBuff.setTextSize(1);

    // Time
    screenBuff.drawString(getTimeString(), 4, 4);

    // Battery
    int batt = getBatteryPercent();
    String battStr = String(batt) + "%";
    int battX = screenBuff.width() - screenBuff.textWidth(battStr) - 4;
    screenBuff.drawString(battStr, battX, 4);

    // Signal bars
    unsigned long nowMs = millis();
    if (nowMs - lastSignalUpdate >= 1000) {
        lastSignalUpdate = nowMs;
        cachedRegistered = isSim800Registered();
        cachedBars = getSim800SignalStrength();
    }
    if (cachedRegistered) {
        int barWidth = 3, barSpacing = 2, baseX = battX - (barWidth + barSpacing) * 4 - 8;
        for (int i = 0; i < 4; i++) {
            int h = (i + 1) * 3;
            uint16_t color = (i < cachedBars) ? TFT_WHITE : TFT_DARKGREY;
            screenBuff.fillRect(baseX + i * (barWidth + barSpacing), 16 - h, barWidth, h, color);
        }
    } else {
        int size = 8;
        int x = battX - size - 4;
        int y = 4;
        screenBuff.drawLine(x, y, x + size, y + size, TFT_RED);
        screenBuff.drawLine(x + size, y, x, y + size, TFT_RED);
    }
}
