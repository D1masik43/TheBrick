#include "nonStaticPrograms/dummy.h"
#include "staticPrograms/mainMenu.h"

// Optionally define a static icon
static const uint16_t dummy_icon[16 * 16 / 8] = { 0 };  // blank 16x16 monochrome

DummyNonStaticApp::DummyNonStaticApp(const std::string& name)
    : NonStaticApp(name) {}

void DummyNonStaticApp::Loop() {
    Draw();
}

void DummyNonStaticApp::UpdateButtons(int button) {
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

void DummyNonStaticApp::UpdateTouch(const TouchPoint* touches, int count) {
    // Nothing
}

void DummyNonStaticApp::Setup() {
    screenBuff = &SystemDrivers::Get().GetScreenBuff();
}

void DummyNonStaticApp::Draw() {
    screenBuff->fillRectVGradient(0, 0, 240, 320, TFT_RED, TFT_WHITE);
}

const uint16_t* DummyNonStaticApp::getIcon() {
    return dummy_icon;
}

const uint16_t* DummyNonStaticApp::StaticIcon() {
    return dummy_icon;
}
