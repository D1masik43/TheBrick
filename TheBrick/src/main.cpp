#include "System/systemDrivers.h"
#include "System/buttonHandler.h"
#include "staticPrograms/mainMenu.h"
#include "appTemplates/staticApp.h"
#include "System/systemCommon.h"

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

  SystemCommon::Get().GetCurrentApp()->Loop();
  int buttonIndex;
    if (xQueueReceive(buttonEventQueue, &buttonIndex, 0))
    {
      Serial.println(buttonIndex);
      SystemCommon::Get().GetCurrentApp()->UpdateButtons(buttonIndex);
    }

 TouchPoint receivedPoints[2];

  if (xQueueReceive(touchEventQueue, &receivedPoints, 0)) {
      int count = 0;
      if (receivedPoints[0].type != NONE) count++;
      if (receivedPoints[1].type != NONE) count++;

      SystemCommon::Get().GetCurrentApp()->UpdateTouch(receivedPoints, count);
  }


  screenBuff->pushSprite(0, 0);

  SystemCommon::Get().ProcessAppSwitch();
}
