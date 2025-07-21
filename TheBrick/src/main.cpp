#include "System/systemDrivers.h"
#include "System/buttonHandler.h"
#include "staticPrograms/mainMenu.h"
#include "appTemplates/staticApp.h"

StaticApp *currentApp = &MainMenu::Get();
void setup() {
SystemDrivers::Get().Setup();
currentApp->Setup();
}

void loop() {
 currentApp->Loop();
  int buttonIndex;
    if (xQueueReceive(buttonEventQueue, &buttonIndex, 0))
    {

        // apps
        currentApp->UpdateButtons(buttonIndex);
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

        currentApp->UpdateTouch(points, valid);
    } 
}
