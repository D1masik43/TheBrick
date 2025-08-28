#pragma once
#include <TFT_eSPI.h>
#include <RTClib.h>

class SystemDrivers;

class StatusBar {
public:
    static StatusBar& Get();

    void Setup();
    void Draw(TFT_eSprite& sprite, bool inMenu, uint16_t bg_color);

private:
    StatusBar() = default;
    StatusBar(const StatusBar&) = delete;
    StatusBar& operator=(const StatusBar&) = delete;

    String getTimeString();
    int getBatteryPercent();
    int getSim800SignalStrength();
    bool isSim800Registered();

    RTC_DS3231* rtc = nullptr;

    String cachedTime = "--:--";
    unsigned long lastTimeUpdate = 0;

    int cachedBars = 0;
    bool cachedRegistered = false;
    unsigned long lastSignalUpdate = 0;
};
