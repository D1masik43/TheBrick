#pragma once
#include <stdint.h>
#include <esp_attr.h>
#include <Arduino.h>

extern const uint16_t defaultWallpaper[320][240];

extern uint16_t (*wallpaper)[240];
extern uint16_t (*wallpaperBlurred)[240];

namespace Icons{
extern const uint16_t Dummy[48][48];
extern const uint16_t Camera[48][48];

};