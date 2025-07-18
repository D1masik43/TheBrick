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

}

void MainMenu::UpdateTouch(const TouchPoint* touches, int count) {

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


