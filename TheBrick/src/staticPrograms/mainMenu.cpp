#include "staticPrograms/mainMenu.h"

MainMenu &MainMenu::Get(std::string name) {
    static MainMenu instance(name);
    return instance;
}

MainMenu::MainMenu(std::string name) : StaticApp(name) {

}

void MainMenu::Loop() {
    Draw();
}

void MainMenu::UpdateButtons(int button) {
    Serial.println(button);                                          //  DEBUG
}

void MainMenu::UpdateTouch(const TouchPoint* touches, int count) {
     if (touches == nullptr || count == 0) {                         //  DEBUG
        return;
    }
    Serial.printf("Touches count: %d\n", count);                     //  DEBUG
    for (int i = 0; i < count; i++) {
        Serial.printf("Touch %d: x=%d, y=%d\n", i, touches[i].x, touches[i].y);
    }
}

void MainMenu::Setup() {
    tft = &SystemDrivers::Get().GetTFT();
}

void MainMenu::Draw() {
    tft->fillScreen(TFT_RED);
}


const unsigned char *MainMenu::getIcon() {
    return nullptr;
}


