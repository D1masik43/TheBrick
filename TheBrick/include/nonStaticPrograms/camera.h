#pragma once

#include "appTemplates/nonStaticApp.h"
#include "System/systemGlobals.h"
#include <esp_camera.h>

// === Camera Pin Mapping ===
#define PWDN_GPIO_NUM  -1
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM  15
#define SIOD_GPIO_NUM  4
#define SIOC_GPIO_NUM  5

#define Y2_GPIO_NUM 11
#define Y3_GPIO_NUM 9
#define Y4_GPIO_NUM 8
#define Y5_GPIO_NUM 10
#define Y6_GPIO_NUM 12
#define Y7_GPIO_NUM 18
#define Y8_GPIO_NUM 17
#define Y9_GPIO_NUM 16

#define VSYNC_GPIO_NUM 6
#define HREF_GPIO_NUM  7
#define PCLK_GPIO_NUM  13

class Mainmenu;
class Appmenu;

class CameraNonStaticApp : public NonStaticApp {
public:
    CameraNonStaticApp(const std::string& name);

    void Loop() override;
    void UpdateButtons(int button) override;
    void UpdateTouch(const TouchPoint* touches, int count) override;
    void Setup() override;
    void Draw() override;
    const uint16_t* getIcon() override;

    static const uint16_t* StaticIcon(); 

    void CloseApp() override;

private:
    TFT_eSprite *screenBuff;
};
    