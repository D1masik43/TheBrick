#include "staticPrograms/appMenu.h"

AppMenu &AppMenu::Get(std::string name) {
    static AppMenu instance(name);
    return instance;
}

AppMenu::AppMenu(std::string name) : StaticApp(name) {

}

void AppMenu::Loop() {
    Draw();
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

int rectX = 100;
int rectY = 200;
int rectRadius = 24;

void AppMenu::UpdateTouch(const TouchPoint* touches, int count) {
    if(touches[0].type == SLIDE)
    {
        if(((touches[0].x > rectX-rectRadius) || (touches[0].x < rectX+rectRadius)) && ((touches[0].y > rectY-rectRadius) | (touches[0].y < rectY+rectRadius)))
        {
            rectX = touches[0].x;
            rectY = touches[0].y;
        }
    }
}

void AppMenu::Setup() {
    screenBuff = &SystemDrivers::Get().GetScreenBuff();

}

void AppMenu::DrawBlurredPatch(int x0, int y0, int w, int h) {
    for (int y = y0; y < y0 + h; y++) {
        for (int x = x0; x < x0 + w; x++) {
            if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
                uint16_t p = wallpaperBlurred[y][x];
                screenBuff->drawPixel(x, y, p);
            }
        }
    }
}

void AppMenu::Draw() {
    
    DrawBlurredPatch(0, 0, 240, 320);
    screenBuff->fillRect(rectX-rectRadius, rectY-rectRadius, rectRadius*2, rectRadius*2, TFT_RED);
    screenBuff->pushImage(rectX-rectRadius, rectY-rectRadius, rectRadius*2, rectRadius*2, DummyAppNonStaticAppWrapper::Get().getIcon());
}



const uint16_t *AppMenu::getIcon() {
    return nullptr;
}