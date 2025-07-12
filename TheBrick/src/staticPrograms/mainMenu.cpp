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

void MainMenu::Update(int button) {

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


