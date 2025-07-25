#include "staticPrograms/mainMenu.h"

MainMenu &MainMenu::Get(std::string name) {
    static MainMenu instance(name);
    return instance;
}

MainMenu::MainMenu(std::string name) : StaticApp(name) {

}

void MainMenu::Loop() {
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
     if (touches == nullptr || count == 0) {
        return;
    }

}

void MainMenu::Setup() {
    screenBuff = &SystemDrivers::Get().GetScreenBuff();

}

void MainMenu::DrawBlurredPatch(int x0, int y0, int w, int h, int blurIntensity) {
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

void MainMenu::Draw() {
    screenBuff->pushImage(0, 0, 240, 320, (const uint16_t*)wallpaper);
    
    DrawBlurredPatch(0, 0, 240, 20, 6);
     size_t free_dram = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
    size_t free_psram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);

    screenBuff->setTextColor(TFT_WHITE,TFT_BLACK);
    screenBuff->setCursor(6, 4);
    screenBuff->setTextSize(1);
    screenBuff->printf("DRAM: %d KB\n PSRAM: %d KB", (int)free_dram/1024, (int)free_psram/1024);
}



const unsigned char *MainMenu::getIcon() {
    return nullptr;
}


