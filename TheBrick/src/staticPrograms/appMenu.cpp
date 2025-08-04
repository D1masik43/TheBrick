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

void AppMenu::ScrollToSelected() {
    int iconY = selectedCol * paddingIcons + startPoint;

    // Center the icon: offset needed to bring icon to vertical center
    int centeredOffset = -(iconY - (SCREEN_HEIGHT / 2 - iconSize / 2));

    // Clamp to scroll limits
    if (centeredOffset < -maxOffsetY) centeredOffset = -maxOffsetY;
    if (centeredOffset > minOffsetY) centeredOffset = minOffsetY;

    totalOffsetY = centeredOffset;
}



void AppMenu::UpdateButtons(int button) {
    bool changed = false;

    switch(button) {
        case BUTTON_UP:
            if (selectedCol > 0) { selectedCol--; changed = true; }
            break;
        case BUTTON_DOWN:
            if (selectedCol < 5) { selectedCol++; changed = true; }
            break;
        case BUTTON_LEFT:
            if (selectedRow > 0) { selectedRow--; changed = true; }
            break;
        case BUTTON_RIGHT:
            if (selectedRow < 2) { selectedRow++; changed = true; }
            break;
        case BUTTON_IN:
            SystemCommon::Get().SetNextApp(appList[selectedRow][selectedCol]);
            break;
        case BUTTON_BACK:
        case BUTTON_HOME:
            SystemCommon::Get().SetNextApp(&MainMenu::Get());
            break;
        default:
            break;
    }

    if (changed) {
        ScrollToSelected();
    }
}



void AppMenu::CheckAppTap(int xTouch, int yTouch) {
    for (int col = 0; col < 6; col++) {
        for (int row = 0; row < 3; row++) {
            int x = row * paddingIcons + paddingX;
            int y = col * paddingIcons + totalOffsetY + startPoint;

            if (xTouch >= x && xTouch < x + iconSize &&
                yTouch >= y && yTouch < y + iconSize) {

                SystemCommon::Get().SetNextApp(appList[row][col]);
                return;
            }
        }
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

        if (totalOffsetY < -maxOffsetY) totalOffsetY = -maxOffsetY;
        if (totalOffsetY > minOffsetY) totalOffsetY = minOffsetY;
    }
    } else if (touches[0].type == SLIDE_END) {
        isSliding = false;
    } else if (touches[0].type == TAP) {
        CheckAppTap(touches[0].x, touches[0].y);
    }
}

void AppMenu::Setup() {
    screenBuff = &SystemDrivers::Get().GetScreenBuff();

    int totalCols = 6;
    int visibleHeight = SCREEN_HEIGHT - topPadding - bottomPadding;
    int contentHeight = totalCols * paddingIcons;
    
    maxOffsetY = std::max(0, contentHeight - visibleHeight);
}


void AppMenu::Draw() {
    screenBuff->pushImage(0, startPoint-36, SCREEN_WIDTH, SCREEN_HEIGHT, (const uint16_t*)wallpaperBlurred);

    for (int col = 0; col < 6; col++) {
        for (int row = 0; row < 3; row++) {
            int x = row * paddingIcons + paddingX;
            int y = col * paddingIcons + totalOffsetY + startPoint;
            screenBuff->pushImage(x, y, iconSize, iconSize, appList[row][col]->getIcon());
        }
    }

    for (int col = 0; col < 6; col++) {
        for (int row = 0; row < 3; row++) {
            int x = row * paddingIcons + paddingX;
            int y = col * paddingIcons + totalOffsetY + startPoint;

            screenBuff->pushImage(x, y, iconSize, iconSize, appList[row][col]->getIcon());

            if (row == selectedRow && col == selectedCol) {
                screenBuff->drawRect(x - 2, y - 2, iconSize + 4, iconSize + 4, TFT_YELLOW);
                screenBuff->drawRect(x - 3, y - 3, iconSize + 6, iconSize + 6, TFT_YELLOW);
            }
        }
    }
    
    screenBuff->pushImage(0, startPoint-36, SCREEN_WIDTH, 32, (const uint16_t*)wallpaperBlurred);
    int wallpaperOffsetY = SCREEN_HEIGHT - 20;
    const uint16_t* bottomPart = (const uint16_t*)wallpaperBlurred + wallpaperOffsetY * SCREEN_WIDTH;
    screenBuff->pushImage(0, SCREEN_HEIGHT - 17, SCREEN_WIDTH, 20, bottomPart);

    

    screenBuff->setTextColor(TFT_GREEN, TFT_BLACK);
    screenBuff->setCursor(5, 5);
    screenBuff->setTextSize(1);
    screenBuff->printf("FPS: %.1f", currentFPS);
}

const uint16_t *AppMenu::getIcon() {
    return nullptr;
}

void AppMenu::setStartPoint(int newPoint)
{
    startPoint = newPoint;
}