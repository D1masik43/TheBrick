#pragma once

#include "appTemplates/nonStaticApp.h"
#include "System/systemGlobals.h"

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

private:
    TFT_eSprite *screenBuff;
};
    