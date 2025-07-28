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
