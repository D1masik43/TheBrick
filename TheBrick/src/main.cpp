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
}
