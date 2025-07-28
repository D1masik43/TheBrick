#pragma once
#include <stdint.h>
#include <esp_attr.h>

extern const uint16_t defaultWallpaper[320][240];

extern uint16_t (*wallpaper)[240];
extern uint16_t (*wallpaperBlurred)[240];
