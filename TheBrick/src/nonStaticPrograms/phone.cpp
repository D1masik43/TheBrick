#include "nonStaticPrograms/phone.h"
#include "staticPrograms/mainMenu.h"

// Optionally define a static icon
static const uint16_t camera_icon[16 * 16 / 8] = { 0 };  // blank 16x16 monochrome

PhoneNonStaticApp::PhoneNonStaticApp(const std::string& name)
    : NonStaticApp(name) {}

void PhoneNonStaticApp::Loop() {
    Draw();
}

void PhoneNonStaticApp::UpdateButtons(int button) {
    switch(button) {
        case BUTTON_UP:

            break;
        case BUTTON_DOWN:

            break;
        case BUTTON_LEFT:

            break;
        case BUTTON_RIGHT:

            break;
        case BUTTON_IN:
            break;
        case BUTTON_BACK:
            SystemCommon::Get().SetNextApp(&AppMenu::Get());
            break;
        case BUTTON_HOME:
            SystemCommon::Get().SetNextApp(&MainMenu::Get());
            break;
        case BUTTON_KEY1:
            sim800->println("ATA");
            break;
        case BUTTON_KEY2:
            sim800->println("ATH");
            break;
        default:

        break;
    }
}

void PhoneNonStaticApp::UpdateTouch(const TouchPoint* touches, int count) {
    // Nothing
}

void PhoneNonStaticApp::Setup() {
  screenBuff = &SystemDrivers::Get().GetScreenBuff();
  sim800 = &SystemDrivers::Get().GetSim800();
}

void PhoneNonStaticApp::Draw() {

}

const uint16_t* PhoneNonStaticApp::getIcon() {
    return camera_icon;
}

const uint16_t* PhoneNonStaticApp::StaticIcon() {
    return camera_icon;
}

void PhoneNonStaticApp::CloseApp() {

}