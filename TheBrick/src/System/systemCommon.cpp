#include "System/systemCommon.h"
#include "staticPrograms/mainMenu.h"

AppBase* SystemCommon::currentApp = &MainMenu::Get();
AppBase* SystemCommon::nextApp = nullptr; 

SystemCommon &SystemCommon::Get(std::string name) {
    static SystemCommon instance(name);
    return instance;
}

SystemCommon::SystemCommon(std::string name) : StaticApp(name) {

}

void SystemCommon::Loop() {

}

void SystemCommon::UpdateButtons(int button) {

}

void SystemCommon::UpdateTouch(const TouchPoint* touches, int count) {

}

void SystemCommon::Setup() {

}

void SystemCommon::Draw() {

}


AppBase* SystemCommon::GetCurrentApp() {
    return currentApp;
}

void SystemCommon::SetNextApp(AppBase* app) {
    nextApp = app;
}

void SystemCommon::ProcessAppSwitch() {
    if (nextApp != nullptr) {
        currentApp = nextApp;
        currentApp->Setup();
        nextApp = nullptr;
    }
}

const uint16_t *SystemCommon::getIcon() {
    return nullptr;
}