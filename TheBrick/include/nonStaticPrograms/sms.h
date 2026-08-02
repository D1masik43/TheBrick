#pragma once
#include "appTemplates/nonStaticApp.h"
#include "System/systemGlobals.h"
#include "System/SystemUI/Scrollable.h"
#include "System/SystemUI/Keyboard.h" it 
#include <vector>

class MainMenu;
class AppMenu;

struct SmsEntry {
    int index;
    String sender;
    String date;
    String body;
};

enum SmsView {
    SMS_LIST,
    SMS_DETAIL,
    SMS_CONFIRM_DELETE
};

class SmsNonStaticApp : public NonStaticApp {
public:
    SmsNonStaticApp(const std::string& name);

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

    std::vector<SmsEntry> messages;
    int selectedIdx = 0;
    Scrollable scroll;
    SmsView view = SMS_LIST;

    void fetchMessages();
    String sim800Cmd(const char* cmd, int waitMs = 500);
    void parseCMGL(const String& raw);
    void deleteMessage(int simIndex);

    void drawList();
    void drawDetail();
    void drawConfirm();
    void drawLoading();

    int itemHeight = 40;
    bool loading = false;
};
