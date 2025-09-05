#pragma once
#include "appTemplates/nonStaticApp.h"
#include "System/systemGlobals.h"
#include <vector>

class MainMenu;
class AppMenu;

enum CallState {
    CALL_IDLE,
    CALL_DIALING,
    CALL_IN_CALL
};

class PhoneNonStaticApp : public NonStaticApp {
public:
    PhoneNonStaticApp(const std::string& name);

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
    HardwareSerial *sim800;

    std::vector<UIButton> buttons;
    std::string dialedNumber;
    CallState callState;

    void initButtons();
    void handlePress(int id);
};
