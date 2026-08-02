#include "System/systemDrivers.h"
#include "System/buttonHandler.h"
#include "staticPrograms/mainMenu.h"
#include "staticPrograms/appMenu.h"
#include "appTemplates/staticApp.h"
#include "System/systemCommon.h"
#include "System/SystemUI/QuickSettings.h"

TFT_eSprite *screenBuff;

void MakeBlurredWalpaper(int x0, int y0, int w, int h, int blurIntensity) {
    for (int y = y0; y < y0 + h; y++) {
        for (int x = x0; x < x0 + w; x++) {
            int r = 0, g = 0, b = 0, count = 0;
            for (int dy = -blurIntensity; dy <= blurIntensity; dy++) {
                for (int dx = -blurIntensity; dx <= blurIntensity; dx++) {
                    int px = x + dx;
                    int py = y + dy;
                    if (px >= 0 && px < SCREEN_WIDTH && py >= 0 && py < SCREEN_HEIGHT) {
                        uint16_t p = wallpaper[py][px];
                        p = (p >> 8) | (p << 8);
                        r += (p >> 11) & 0x1F;
                        g += (p >> 5) & 0x3F;
                        b += p & 0x1F;
                        count++;
                    }
                }
            }
            r /= count;
            g /= count;
            b /= count;
            uint16_t blurred = (r << 11) | (g << 5) | b;
            blurred = (blurred >> 8) | (blurred << 8);
            wallpaperBlurred[y][x] = blurred;
        }
    }
}


void setup() {
  screenBuff = &SystemDrivers::Get().GetScreenBuff();

  SystemDrivers::Get().Setup();
  SystemCommon::Get().GetCurrentApp()->Setup();
  AppMenu::Get().Setup();

    wallpaper = (uint16_t (*)[240]) heap_caps_malloc(320 * 240 * sizeof(uint16_t), MALLOC_CAP_SPIRAM);
    if (!wallpaper) {
        Serial.println("Failed to allocate wallpaper in PSRAM");
        return;
    }
    // Copy from flash (initialized wallpaper) to PSRAM buffer
    memcpy(wallpaper, defaultWallpaper, 320 * 240 * sizeof(uint16_t));

    wallpaperBlurred = (uint16_t (*)[240]) heap_caps_malloc(320 * 240 * sizeof(uint16_t), MALLOC_CAP_SPIRAM);
    if (!wallpaperBlurred) {
        Serial.println("Failed to allocate wallpaper in PSRAM");
        return;
    }
    MakeBlurredWalpaper(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 6);

    StatusBar::Get().Setup();
}

void loop() {
    if (Serial.available() > 0) {
    Serial.println("Pong");
    String input = Serial.readStringUntil('\n');
    int y, m, d, hh, mm, ss;
    // eg. "26:03:22:23:01:00" or "2026/03/22 23:01:00"
    if (sscanf(input.c_str(), "%d %*c %d %*c %d %*c %d %*c %d %*c %d", &y, &m, &d, &hh, &mm, &ss) == 6) {
      if (y < 100) y += 2000; 
      
      if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
          SystemDrivers::Get().GetRTC().adjust(DateTime(y, m, d, hh, mm, ss));
          xSemaphoreGive(i2cMutex);
      }
      Serial.println("RTC Updated!");
    }
  }
  SystemCommon::Get().GetCurrentApp()->Loop();
  int buttonIndex;
    if (xQueueReceive(buttonEventQueue, &buttonIndex, 0))
    {
      Serial.println(buttonIndex);
      SystemCommon::Get().GetCurrentApp()->UpdateButtons(buttonIndex);
    }

 TouchPoint receivedPoints[2];
  static int swipeStartX = 0;
  QuickSettings& qs = QuickSettings::Get();

  if (xQueueReceive(touchEventQueue, &receivedPoints, 0)) {
      int count = 0;
      if (receivedPoints[0].type != NONE) count++;
      if (receivedPoints[1].type != NONE) count++;

      if (qs.IsOpen()) {
          qs.UpdateTouch(receivedPoints, count);
          goto skipTouch;
      }

      qs.CheckSwipeOpen(receivedPoints[0]);
      if (qs.IsOpen() || qs.IsAnimating()) goto skipTouch;

      if (receivedPoints[0].type == SLIDE_BEGIN) {
          swipeStartX = receivedPoints[0].x;
      } else if (receivedPoints[0].type == SLIDE_END) {
          int deltaX = receivedPoints[0].x - swipeStartX;
          if (swipeStartX < 40 && deltaX > 60) {
              SystemCommon::Get().GetCurrentApp()->UpdateButtons(BUTTON_BACK);
              goto skipTouch;
          }
      }

      SystemCommon::Get().GetCurrentApp()->UpdateTouch(receivedPoints, count);
      skipTouch:;
  }

  AppBase* currentApp = SystemCommon::Get().GetCurrentApp();
  bool isMenu = (currentApp == (AppBase*)&MainMenu::Get() || currentApp == (AppBase*)&AppMenu::Get());
  StatusBar::Get().Draw(*screenBuff, isMenu, TFT_BLACK);

  if (qs.IsOpen() || qs.IsAnimating())
      qs.Draw(*screenBuff);

  screenBuff->pushSprite(0, 0);

  SystemCommon::Get().ProcessAppSwitch();
}