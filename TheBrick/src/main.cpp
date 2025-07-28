#include "System/systemDrivers.h"
#include "System/buttonHandler.h"
#include "staticPrograms/mainMenu.h"
#include "appTemplates/staticApp.h"
#include "System/systemCommon.h"

TouchPoint lastPoints[2];
bool lastTouchValid[2] = { false, false };

TFT_eSprite *screenBuff;

void setup() {
  screenBuff = &SystemDrivers::Get().GetScreenBuff();

  SystemDrivers::Get().Setup();
  SystemCommon::Get().GetCurrentApp()->Setup();

 wallpaper = (uint16_t (*)[240]) heap_caps_malloc(320 * 240 * sizeof(uint16_t), MALLOC_CAP_SPIRAM);
    if (!wallpaper) {
        Serial.println("Failed to allocate wallpaper in PSRAM");
        return;
    }
    // Copy from flash (initialized wallpaper) to PSRAM buffer
  memcpy(wallpaper, defaultWallpaper, 320 * 240 * sizeof(uint16_t));
}

void loop() {

  SystemCommon::Get().GetCurrentApp()->Loop();
  int buttonIndex;
    if (xQueueReceive(buttonEventQueue, &buttonIndex, 0))
    {
      Serial.println(buttonIndex);
      SystemCommon::Get().GetCurrentApp()->UpdateButtons(buttonIndex);
    }

 arduino::ft6336<SCREEN_WIDTH, SCREEN_HEIGHT>& touch = SystemDrivers::Get().GetTouch();

  if (touch.update()) {
    size_t count = touch.touches();
    TouchPoint points[2] = {};  // Reset each loop
    bool currentTouchValid[2] = { false, false };

    uint16_t x, y;

    // First touch
    if (touch.xy(&x, &y)) {
      currentTouchValid[0] = true;

      TouchType type = NONE;
      if (lastTouchValid[0]) type = SLIDE;

      points[0] = { static_cast<int>(x), static_cast<int>(y), type };
    } else {
      if (lastTouchValid[0]) {
        // Previously touched, now released = TAP
        points[0] = { lastPoints[0].x, lastPoints[0].y, TAP };
        currentTouchValid[0] = false;
      }
    }

    // Second touch
    if (count > 1 && touch.xy2(&x, &y)) {
      currentTouchValid[1] = true;

      TouchType type = NONE;
      if (lastTouchValid[1]) type = SLIDE;

      points[1] = { static_cast<int>(x), static_cast<int>(y), type };
    } else {
      if (lastTouchValid[1]) {
        points[1] = { lastPoints[1].x, lastPoints[1].y, TAP };
        currentTouchValid[1] = false;
      }
    }

    // Count how many valid touches to pass
    int validCount = (points[0].type != NONE) + (points[1].type != NONE);
    SystemCommon::Get().GetCurrentApp()->UpdateTouch(points, validCount);

    // Save last touch state
    lastPoints[0] = points[0];
    lastPoints[1] = points[1];
    lastTouchValid[0] = currentTouchValid[0];
    lastTouchValid[1] = currentTouchValid[1];
  }

  screenBuff->pushSprite(0, 0);

  SystemCommon::Get().ProcessAppSwitch();
}
