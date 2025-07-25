#include "System/systemDrivers.h"
#include "System/buttonHandler.h"
#include "staticPrograms/mainMenu.h"
#include "appTemplates/staticApp.h"
#include "System/systemCommon.h"

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
    TouchPoint points[2];  // Max 2 touches supported

    int valid = 0;
    uint16_t x, y;

    if (touch.xy(&x, &y)) {
      points[valid++] = {static_cast<int>(x), static_cast<int>(y)};
    }

    if (count > 1 && touch.xy2(&x, &y)) {
      points[valid++] = {static_cast<int>(x), static_cast<int>(y)};
    }

    SystemCommon::Get().GetCurrentApp()->UpdateTouch(points, valid);
  } 

  screenBuff->pushSprite(0, 0);

  SystemCommon::Get().ProcessAppSwitch();
}
