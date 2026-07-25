#pragma once

#include "appTemplates/nonStaticApp.h"
#include "System/systemGlobals.h"
#include "System/SystemUI/Scrollable.h"

class Mainmenu;
class Appmenu;

class SysInfoNonStaticApp : public NonStaticApp {
public:
    SysInfoNonStaticApp(const std::string& name);

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
    Scrollable scroll;
    unsigned long lastUpdate = 0;

    float inaVoltage = 0;
    float inaCurrent = 0;
    float inaPower = 0;
    float inaShunt = 0;

    uint8_t i2cAddrs[16];
    int i2cCount = 0;
};
