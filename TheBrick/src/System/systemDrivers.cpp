#include "System/systemDrivers.h"

SystemDrivers &SystemDrivers::Get(std::string name) {
    static SystemDrivers instance(name);
    return instance;
}

TFT_eSPI &SystemDrivers::GetTFT() {
    static TFT_eSPI tft;
    return tft;
}

SystemDrivers::SystemDrivers(std::string name) : StaticApp(name) {

}

void SystemDrivers::Loop() {

}

void SystemDrivers::Update(int button) {

}

void SystemDrivers::Setup() {
  TFT_eSPI *tft = &GetTFT();
  tft->begin();
  tft->setRotation(3);
  tft->invertDisplay(true);
  tft->fillScreen(TFT_BLACK);
}

void SystemDrivers::Draw() {

}


const unsigned char *SystemDrivers::getIcon() {
    return nullptr;
}


