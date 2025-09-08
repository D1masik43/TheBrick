#include "nonStaticPrograms/files.h"
#include "staticPrograms/mainMenu.h"

FilesNonStaticApp::FilesNonStaticApp(const std::string& name)
    : NonStaticApp(name) {}

void FilesNonStaticApp::Loop() {
    Draw();
}

void FilesNonStaticApp::UpdateButtons(int button) {
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

void FilesNonStaticApp::UpdateTouch(const TouchPoint* touches, int count) {

}

void FilesNonStaticApp::Setup() {
    screenBuff = &SystemDrivers::Get().GetScreenBuff();
}

void FilesNonStaticApp::Draw() {
  
}

void FilesNonStaticApp::CloseApp() {

}