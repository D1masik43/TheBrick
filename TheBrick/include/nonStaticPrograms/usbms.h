#pragma once
#include "appTemplates/nonStaticApp.h"
#include "System/systemGlobals.h"

class MainMenu;
class AppMenu;

class UsbmsNonStaticApp : public NonStaticApp {
public:
    UsbmsNonStaticApp(const std::string& name);

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
    bool connected = false;

    UIButton connectBtn = UIButton(1, 50, 140, 140, 50, "Connect");
    UIButton disconnectBtn = UIButton(2, 50, 140, 140, 50, "Disconnect");

    void startMSC();
    void stopMSC();
};
