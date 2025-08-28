#pragma once
#include <TFT_eSPI.h>
#include <RTClib.h>

class StatusBar {
public:
    static StatusBar& Get();
    void Setup();
    void Draw(TFT_eSprite& screenBuff, bool inMenu, uint16_t bg_color);

private:
    StatusBar() = default;
    StatusBar(const StatusBar&) = delete;
    StatusBar& operator=(const StatusBar&) = delete;

    RTC_DS3231* rtc = nullptr;

    // cached values
    String cachedTime = "--:--";
    int cachedBattery = -1;
    int cachedBars = 0;
    bool cachedRegistered = false;

    // internal
    void statusTask(void* param);
};