#include "staticPrograms/appMenu.h"

AppMenu &AppMenu::Get(std::string name) {
    static AppMenu instance(name);
    return instance;
}

AppMenu::AppMenu(std::string name) : StaticApp(name) {

}

void AppMenu::Loop() {
    uint32_t now = millis();
    Draw();
    uint32_t frameTime = millis() - now;
    if (frameTime > 0) {
        currentFPS = 1000.0f / frameTime;
    }
}


void AppMenu::UpdateButtons(int button) {
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
            SystemCommon::Get().SetNextApp(&MainMenu::Get());
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

void AppMenu::UpdateTouch(const TouchPoint* touches, int count) {
    if (touches[0].type == SLIDE_BEGIN) {
        slideStartY = touches[0].y;
        lastSlideY = touches[0].y;
        isSliding = true;
    } else if (touches[0].type == SLIDE) {
        if (isSliding) {
            int delta = touches[0].y - lastSlideY;
            totalOffsetY += delta;
            lastSlideY = touches[0].y;
        }
    } else if (touches[0].type == SLIDE_END) {
        isSliding = false;
    }
}



void AppMenu::Setup() {
    screenBuff = &SystemDrivers::Get().GetScreenBuff();

}

static uint16_t lineBuf[SCREEN_WIDTH]; // in internal RAM

void AppMenu::Draw() {
        screenBuff->pushImage(0, 0, 240, 320, (const uint16_t*)wallpaperBlurred);

    for (int col = 0; col < 6; col++) {
        for (int row = 0; row < 3; row++) {
            int x = row * paddingIcons + paddingX;
            int y = col * paddingIcons + totalOffsetY + startPoint;
            screenBuff->pushImage(x, y, iconSize, iconSize, appList[row][col]->getIcon());
        }
    }

    screenBuff->setTextColor(TFT_GREEN, TFT_BLACK);
    screenBuff->setCursor(5, 5);
    screenBuff->setTextSize(1);
    screenBuff->printf("FPS: %.1f", currentFPS);
}

const uint16_t *AppMenu::getIcon() {
    return nullptr;
}