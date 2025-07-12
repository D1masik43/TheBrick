#include "System/systemDrivers.h"
#include "staticPrograms/mainMenu.h"
#include "appTemplates/staticApp.h"

StaticApp *curentApp = &MainMenu::Get();
void setup() {
SystemDrivers::Get().Setup();
curentApp->Setup();
}

void loop() {
 curentApp->Loop();
}
