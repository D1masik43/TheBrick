#include "nonStaticPrograms/phone.h"
#include "staticPrograms/mainMenu.h"
#include "staticPrograms/appMenu.h"

static const uint16_t phone_icon[16 * 16 / 8] = { 0 };

PhoneNonStaticApp::PhoneNonStaticApp(const std::string& name)
    : NonStaticApp(name), dialedNumber(""), callState(CALL_IDLE) {}

void PhoneNonStaticApp::Setup() {
    screenBuff = &SystemDrivers::Get().GetScreenBuff();
    sim800 = &SystemDrivers::Get().GetSim800();
    initButtons();
}

void PhoneNonStaticApp::initButtons() {
    buttons.clear();

    // Number pad layout (3x4)
    const char* labels[12] = {"1","2","3","4","5","6","7","8","9","*","0","#"};
    int id = 0;
    int startX = 35, startY = 48;
    int w = 50, h = 45, pad = 10;

    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 3; col++) {
            int x = startX + col * (w + pad);
            int y = startY + row * (h + pad);
            UIButton btn(id, x, y, w, h, labels[id]);
            btn.SetColors(TFT_WHITE, TFT_DARKGREY, TFT_WHITE);
            buttons.push_back(btn);
            id++;
        }
    }

    int callUI_X = 40;
    int callUI_W = 50;
    int callUI_P = 5;
    // Call button
    UIButton callBtn(100, callUI_X, 45 + 4 * (h + pad), 50, 40, "CALL");
    callBtn.SetColors(TFT_WHITE, TFT_GREEN, TFT_BLACK);
    buttons.push_back(callBtn);

    // Backspace button
    UIButton backBtn(102, callUI_X + callUI_W + callUI_P, 45 + 4 * (h + pad), 50, 40, "<-");
    backBtn.SetColors(TFT_WHITE, TFT_DARKGREY, TFT_WHITE);
    buttons.push_back(backBtn);

    // Hangup button
    UIButton hangBtn(101, callUI_X + callUI_W * 2 + callUI_P * 2, 45 + 4 * (h + pad), 50, 40, "HANG");
    hangBtn.SetColors(TFT_WHITE, TFT_RED, TFT_BLACK);
    buttons.push_back(hangBtn);
}

void PhoneNonStaticApp::Loop() {
    Draw();
}

void PhoneNonStaticApp::Draw() {
    screenBuff->fillSprite(TFT_BLACK);

    // Header with number
    screenBuff->setTextDatum(TL_DATUM);
    screenBuff->setTextColor(TFT_WHITE, TFT_BLACK);
    screenBuff->setFreeFont(nullptr);
    String numStr = "+38" + String(dialedNumber.c_str());
    screenBuff->drawString(numStr, 10, 24, 4);


    // Draw buttons
    for (auto& b : buttons) {
        b.Draw(*screenBuff);
    }

    StatusBar::Get().Draw(*screenBuff, false, TFT_BLACK);
}

void PhoneNonStaticApp::UpdateTouch(const TouchPoint* touches, int count) {
    if (count <= 0) return;
    const TouchPoint& tp = touches[0];

    if (tp.type == TAP) {
        for (auto& b : buttons) {
            int id = b.IsPressed(tp);
            if (id != -1) {
                handlePress(id);
                break;
            }
        }
    }
}

void PhoneNonStaticApp::handlePress(int id) {
    if (id < 12) { 
        // Number pressed
        static const char* labels[12] = {"1","2","3","4","5","6","7","8","9","*","0","#"};
        dialedNumber += labels[id];
    } else if (id == 100) {
        // CALL
        if (!dialedNumber.empty()) {
            String cmd = "ATD+38" + String(dialedNumber.c_str()) + ";";
            sim800->println(cmd);
            callState = CALL_DIALING;
        }
        else{
        sim800->println("ATA");
        }
    } else if (id == 101) {
        // HANG
        sim800->println("ATH");
        dialedNumber.clear();
        callState = CALL_IDLE;
    }
    else if (id == 102) {
        if (!dialedNumber.empty()) {
            dialedNumber.pop_back();
        }
    }

}

void PhoneNonStaticApp::UpdateButtons(int button) {
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
            SystemCommon::Get().SetNextApp(&AppMenu::Get());
            break;
        case BUTTON_HOME:
            SystemCommon::Get().SetNextApp(&MainMenu::Get());
            break;
        case BUTTON_KEY1:
            sim800->println("ATA");
            break;
        case BUTTON_KEY2:
            sim800->println("ATH");
            break;
        default:

        break;
    }
}

const uint16_t* PhoneNonStaticApp::getIcon() { return phone_icon; }
const uint16_t* PhoneNonStaticApp::StaticIcon() { return phone_icon; }
void PhoneNonStaticApp::CloseApp() {}
