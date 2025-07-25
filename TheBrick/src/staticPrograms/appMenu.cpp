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

void AppMenu::UpdateTouch(const TouchPoint* touches, int count) {
    if (touches == nullptr || count == 0) {
        return;
    }

}

void AppMenu::Setup() {
    screenBuff = &SystemDrivers::Get().GetScreenBuff();

}

void AppMenu::DrawBlurredPatch(int x0, int y0, int w, int h, int blurIntensity) {
    for (int y = y0; y < y0 + h; y++) {
        for (int x = x0; x < x0 + w; x++) {
            int r = 0, g = 0, b = 0, count = 0;
            for (int dy = -blurIntensity; dy <= blurIntensity; dy++) {
                for (int dx = -blurIntensity; dx <= blurIntensity; dx++) {
                    int px = x + dx;
                    int py = y + dy;
                    if (px >= 0 && px < SCREEN_WIDTH && py >= 0 && py < SCREEN_HEIGHT) {
                        uint16_t p = wallpaper[py][px];
                        p = (p >> 8) | (p << 8); 
                        r += (p >> 11) & 0x1F;
                        g += (p >> 5) & 0x3F;
                        b += p & 0x1F;
                        count++;
                    }
                }
            }
            r /= count;
            g /= count;
            b /= count;
            uint16_t blurred = (r << 11) | (g << 5) | b;
            screenBuff->drawPixel(x, y, blurred);  // ← Sprite version of pushColor
        }
    }
}

void AppMenu::Draw() {
    screenBuff->pushImage(0, 0, 240, 320, (const uint16_t*)wallpaper);\
    
    DrawBlurredPatch(0, 0, 240, 320, 2);

}



const unsigned char *AppMenu::getIcon() {
    return nullptr;
}