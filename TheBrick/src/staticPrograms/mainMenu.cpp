#include "staticPrograms/mainMenu.h"

MainMenu &MainMenu::Get(std::string name) {
    static MainMenu instance(name);
    return instance;
}

MainMenu::MainMenu(std::string name) : StaticApp(name) {

}

void MainMenu::Loop() {
    if (transitioningToAppMenu) {

        transitionY -= 20;
        if (transitionY <= 36) {
            transitionY = 36;
            AppMenu::Get().setStartPoint(36);
            SystemCommon::Get().SetNextApp(&AppMenu::Get());
            transitioningToAppMenu = false;
            return;
        }

        AppMenu::Get().setStartPoint(transitionY);
        AppMenu::Get().Draw();
        return;
    }

    Draw();
}


void MainMenu::UpdateButtons(int button) {
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
            SystemCommon::Get().SetNextApp(&AppMenu::Get());
            break;
        case BUTTON_BACK:

            break;
        case BUTTON_HOME:

            break;
        case BUTTON_KEY1:

            break;
        case BUTTON_KEY2:

            break;
        default:

        break;
    }
}

void MainMenu::UpdateTouch(const TouchPoint* touches, int count) {
    static int startY = 0;

    if (touches[0].type == SLIDE_BEGIN) {
        startY = touches[0].y;
    } else if (touches[0].type == SLIDE_END) {
        int delta = startY - touches[0].y;

        // If started from bottom 50px and moved up > 50px
        if (startY > SCREEN_HEIGHT - 50 && delta > 50) {
            transitioningToAppMenu = true;
            transitionY = SCREEN_HEIGHT;
            AppMenu::Get().setStartPoint(transitionY);
        }
    }
}

void MainMenu::Setup() {
    screenBuff = &SystemDrivers::Get().GetScreenBuff();
}

void MainMenu::Draw() {
    screenBuff->pushImage(0, 0, 240, 320, (const uint16_t*)wallpaper);
    
    screenBuff->pushImage(0, 0, 240, 20, (const uint16_t*)wallpaperBlurred);
    size_t free_dram = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
    size_t free_psram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);

    screenBuff->setTextColor(TFT_WHITE,TFT_BLACK);
    screenBuff->setCursor(6, 4);
    screenBuff->setTextSize(1);
    screenBuff->printf("DRAM: %d KB\n PSRAM: %d KB", (int)free_dram/1024, (int)free_psram/1024);
}



const uint16_t *MainMenu::getIcon() {
    return nullptr;
}


