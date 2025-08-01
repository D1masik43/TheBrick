#include "nonStaticPrograms/camera.h"
#include "staticPrograms/mainMenu.h"

// Optionally define a static icon
static const uint16_t camera_icon[16 * 16 / 8] = { 0 };  // blank 16x16 monochrome

CameraNonStaticApp::CameraNonStaticApp(const std::string& name)
    : NonStaticApp(name) {}

void CameraNonStaticApp::Loop() {
    Draw();
}

void CameraNonStaticApp::UpdateButtons(int button) {
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

            break;
        case BUTTON_KEY2:

            break;
        default:

        break;
    }
}

void CameraNonStaticApp::UpdateTouch(const TouchPoint* touches, int count) {
    // Nothing
}

void CameraNonStaticApp::Setup() {
    screenBuff = &SystemDrivers::Get().GetScreenBuff();
}

void CameraNonStaticApp::Draw() {
    screenBuff->fillRectVGradient(0, 0, 240, 320, TFT_BLUE, TFT_WHITE);
}

const uint16_t* CameraNonStaticApp::getIcon() {
    return camera_icon;
}

const uint16_t* CameraNonStaticApp::StaticIcon() {
    return camera_icon;
}
