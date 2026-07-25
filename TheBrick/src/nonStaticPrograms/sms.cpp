#include "nonStaticPrograms/sms.h"
#include "staticPrograms/mainMenu.h"
#include "staticPrograms/appMenu.h"

static const uint16_t sms_icon[256] = {0};

SmsNonStaticApp::SmsNonStaticApp(const std::string& name)
    : NonStaticApp(name) {}

void SmsNonStaticApp::Setup() {
    screenBuff = &SystemDrivers::Get().GetScreenBuff();
    sim800 = &SystemDrivers::Get().GetSim800();
    loading = true;
}

void SmsNonStaticApp::drawLoading() {
    screenBuff->fillScreen(TFT_BLACK);
    StatusBar::Get().Draw(*screenBuff, false, TFT_BLACK);
    screenBuff->setTextSize(1);
    screenBuff->setTextDatum(MC_DATUM);
    screenBuff->setTextColor(TFT_DARKGREY);
    screenBuff->drawString("Loading messages...", 120, 160);
    screenBuff->setTextDatum(TL_DATUM);
    SystemDrivers::Get().GetTFT().pushImage(0, 0, 240, 320, (uint16_t*)screenBuff->getPointer());
}

String SmsNonStaticApp::sim800Cmd(const char* cmd, int waitMs) {
    if (xSemaphoreTake(sim800Mutex, pdMS_TO_TICKS(waitMs + 500)) != pdTRUE) return "";
    while (sim800->available()) sim800->read();
    sim800->println(cmd);

    String resp = "";
    unsigned long start = millis();
    while (millis() - start < (unsigned long)waitMs) {
        while (sim800->available()) {
            char c = sim800->read();
            if (resp.length() < 8192) resp += c;
        }
        if (resp.indexOf("OK\r") >= 0 || resp.indexOf("ERROR") >= 0) break;
        delay(10);
    }
    xSemaphoreGive(sim800Mutex);
    return resp;
}

static bool isHexString(const String& s) {
    if (s.length() < 4 || s.length() % 2 != 0) return false;
    for (unsigned int i = 0; i < s.length(); i++) {
        char c = s.charAt(i);
        if (!isxdigit(c)) return false;
    }
    return true;
}

static String decodeUCS2(const String& hex) {
    String out = "";
    for (unsigned int i = 0; i + 3 < hex.length(); i += 4) {
        char h[5] = {hex[i], hex[i+1], hex[i+2], hex[i+3], 0};
        uint16_t code = strtol(h, NULL, 16);
        if (code >= 32 && code < 128) {
            out += (char)code;
        } else if (code >= 128) {
            out += '\xA8';
        }
    }
    return out;
}

static String reformatDate(const String& scts) {
    if (scts.length() < 14) return scts;
    // SCTS: "YY/MM/DD,HH:MM:SS+TZ" → "DD.MM HH:MM"
    String dd = scts.substring(6, 8);
    String mm = scts.substring(3, 5);
    String hh = scts.substring(9, 11);
    String mi = scts.substring(12, 14);
    return dd + "." + mm + " " + hh + ":" + mi;
}

void SmsNonStaticApp::fetchMessages() {
    messages.clear();
    selectedIdx = 0;
    scroll.reset();
    sim800Cmd("AT+CMGF=1", 300);
    sim800Cmd("AT+CSCS=\"GSM\"", 300);
    sim800Cmd("AT+CNMI=2,1,0,0,0", 300);

    String cpmsResp = sim800Cmd("AT+CPMS=\"MT\",\"MT\",\"MT\"", 300);
    if (cpmsResp.indexOf("OK") >= 0) {
        String raw = sim800Cmd("AT+CMGL=\"ALL\",1", 15000);
        parseCMGL(raw);
    } else {
        sim800Cmd("AT+CPMS=\"ME\",\"ME\",\"ME\"", 300);
        String raw = sim800Cmd("AT+CMGL=\"ALL\",1", 10000);
        parseCMGL(raw);

        sim800Cmd("AT+CPMS=\"SM\",\"SM\",\"SM\"", 300);
        raw = sim800Cmd("AT+CMGL=\"ALL\",1", 10000);
        parseCMGL(raw);
    }

    loading = false;
}

void SmsNonStaticApp::parseCMGL(const String& raw) {
    int pos = 0;
    while (messages.size() < 50) {
        int hdr = raw.indexOf("+CMGL:", pos);
        if (hdr < 0) break;

        int headerEnd = raw.indexOf('\n', hdr);
        if (headerEnd < 0) break;

        String headerLine = raw.substring(hdr, headerEnd);

        int colonPos = headerLine.indexOf(':');
        int firstComma = headerLine.indexOf(',');
        if (colonPos < 0 || firstComma < 0) break;

        SmsEntry sms;
        sms.index = headerLine.substring(colonPos + 2, firstComma).toInt();

        String quoted[6];
        int qCount = 0;
        int searchFrom = 0;
        while (qCount < 6) {
            int q1 = headerLine.indexOf('"', searchFrom);
            if (q1 < 0) break;
            int q2 = headerLine.indexOf('"', q1 + 1);
            if (q2 < 0) break;
            quoted[qCount++] = headerLine.substring(q1 + 1, q2);
            searchFrom = q2 + 1;
        }

        if (qCount >= 2) sms.sender = quoted[1];
        for (int i = qCount - 1; i >= 2; i--) {
            if (quoted[i].indexOf('/') >= 0 || quoted[i].indexOf(',') >= 0) {
                sms.date = reformatDate(quoted[i]);
                break;
            }
        }

        int bodyStart = headerEnd + 1;

        int nextMsg = raw.indexOf("\r\n+CMGL:", bodyStart);
        int okPos = raw.indexOf("\r\nOK", bodyStart);

        int bodyEnd;
        if (nextMsg >= 0 && (okPos < 0 || nextMsg < okPos))
            bodyEnd = nextMsg;
        else if (okPos >= 0)
            bodyEnd = okPos;
        else
            bodyEnd = raw.length();

        sms.body = raw.substring(bodyStart, bodyEnd);
        sms.body.trim();

        if (isHexString(sms.body)) {
            sms.body = decodeUCS2(sms.body);
        }
        if (isHexString(sms.sender)) {
            sms.sender = decodeUCS2(sms.sender);
        }

        if (sms.sender.length() > 0 || sms.body.length() > 0)
            messages.push_back(sms);

        pos = bodyEnd;
    }
}

void SmsNonStaticApp::deleteMessage(int simIndex) {
    char cmd[20];
    snprintf(cmd, sizeof(cmd), "AT+CMGD=%d", simIndex);
    sim800Cmd(cmd, 1000);
    loading = true;
}

void SmsNonStaticApp::Loop() {
    if (loading) {
        drawLoading();
        fetchMessages();
        return;
    }
    Draw();
}

void SmsNonStaticApp::Draw() {
    screenBuff->fillScreen(TFT_BLACK);

    switch (view) {
        case SMS_LIST: drawList(); break;
        case SMS_DETAIL: drawDetail(); break;
        case SMS_CONFIRM_DELETE: drawConfirm(); break;
    }
}

void SmsNonStaticApp::drawList() {
    screenBuff->setTextSize(1);
    screenBuff->setTextDatum(TL_DATUM);

    if (messages.empty()) {
        screenBuff->setTextColor(TFT_DARKGREY);
        screenBuff->drawString("No messages", 60, 150);
        screenBuff->drawString("[KEY1] Refresh", 60, 170);
        return;
    }

    for (int i = 0; i < (int)messages.size(); i++) {
        int y = 24 + i * itemHeight - scroll.scrollY;
        if (y + itemHeight < 20) continue;
        if (y > SCREEN_HEIGHT) break;

        if (i == selectedIdx) {
            screenBuff->fillRect(0, y, 240, itemHeight - 2, 0x1082);
        }

        screenBuff->setTextColor(TFT_WHITE);
        screenBuff->drawString(messages[i].sender, 6, y + 4);

        screenBuff->setTextColor(TFT_DARKGREY);
        String preview = messages[i].body.substring(0, 30);
        screenBuff->drawString(preview, 6, y + 16);

        screenBuff->setTextColor(TFT_DARKGREY);
        int dateW = screenBuff->textWidth(messages[i].date);
        screenBuff->drawString(messages[i].date, 240 - dateW - 4, y + 4);

        screenBuff->drawLine(0, y + itemHeight - 2, 240, y + itemHeight - 2, 0x2104);
    }
}

void SmsNonStaticApp::drawDetail() {
    if (selectedIdx < 0 || selectedIdx >= (int)messages.size()) return;
    SmsEntry& sms = messages[selectedIdx];

    screenBuff->setTextSize(1);
    screenBuff->setTextDatum(TL_DATUM);

    screenBuff->setTextColor(TFT_CYAN);
    screenBuff->drawString(sms.sender, 6, 26);

    screenBuff->setTextColor(TFT_DARKGREY);
    screenBuff->drawString(sms.date, 6, 38);

    screenBuff->drawLine(0, 50, 240, 50, 0x2104);

    screenBuff->setTextColor(TFT_WHITE);
    int y = 56 - scroll.scrollY;
    int lineH = 12;
    String text = sms.body;
    int start = 0;
    while (start < (int)text.length()) {
        int maxChars = 38;
        int end = start + maxChars;
        if (end > (int)text.length()) end = text.length();

        int nl = text.indexOf('\n', start);
        if (nl >= 0 && nl < end) end = nl + 1;

        String line = text.substring(start, end);
        line.trim();
        if (line.length() > 0 && y >= 50 && y < SCREEN_HEIGHT - 20)
            screenBuff->drawString(line, 6, y);
        y += lineH;
        start = end;
    }

    screenBuff->fillRect(0, SCREEN_HEIGHT - 18, 240, 18, TFT_BLACK);
    screenBuff->setTextColor(TFT_RED);
    screenBuff->setTextDatum(MC_DATUM);
    screenBuff->drawString("[KEY1] Delete", 120, SCREEN_HEIGHT - 9);
    screenBuff->setTextDatum(TL_DATUM);
}

void SmsNonStaticApp::drawConfirm() {
    screenBuff->setTextSize(1);
    screenBuff->setTextDatum(MC_DATUM);
    screenBuff->setTextColor(TFT_WHITE);
    screenBuff->drawString("Delete this message?", 120, 140);

    screenBuff->setTextColor(TFT_GREEN);
    screenBuff->drawString("[IN] Yes", 80, 170);

    screenBuff->setTextColor(TFT_RED);
    screenBuff->drawString("[BACK] No", 160, 170);

    screenBuff->setTextDatum(TL_DATUM);
}

void SmsNonStaticApp::UpdateButtons(int button) {
    if (loading) return;
    switch (view) {
    case SMS_LIST:
        if (button == BUTTON_UP) {
            if (selectedIdx > 0) selectedIdx--;
            int topY = 24 + selectedIdx * itemHeight - scroll.scrollY;
            if (topY < 24) scroll.scrollY = selectedIdx * itemHeight;
        }
        if (button == BUTTON_DOWN) {
            if (selectedIdx < (int)messages.size() - 1) selectedIdx++;
            int botY = 24 + selectedIdx * itemHeight - scroll.scrollY + itemHeight;
            if (botY > SCREEN_HEIGHT) scroll.scrollY = 24 + selectedIdx * itemHeight + itemHeight - SCREEN_HEIGHT;
        }
        if (button == BUTTON_IN && !messages.empty()) {
            view = SMS_DETAIL;
            scroll.reset();
        }
        if (button == BUTTON_BACK) SystemCommon::Get().SetNextApp(&AppMenu::Get());
        if (button == BUTTON_HOME) SystemCommon::Get().SetNextApp(&MainMenu::Get());
        if (button == BUTTON_KEY1) { loading = true; }
        break;

    case SMS_DETAIL:
        if (button == BUTTON_BACK) { view = SMS_LIST; scroll.reset(); }
        if (button == BUTTON_HOME) SystemCommon::Get().SetNextApp(&MainMenu::Get());
        if (button == BUTTON_UP) scroll.scrollY = max(0, scroll.scrollY - 20);
        if (button == BUTTON_DOWN) scroll.scrollY += 20;
        if (button == BUTTON_KEY1) view = SMS_CONFIRM_DELETE;
        break;

    case SMS_CONFIRM_DELETE:
        if (button == BUTTON_IN) deleteMessage(messages[selectedIdx].index);
        if (button == BUTTON_BACK) view = SMS_DETAIL;
        break;
    }
}

void SmsNonStaticApp::UpdateTouch(const TouchPoint* touches, int count) {
    if (count <= 0 || loading) return;
    const TouchPoint& tp = touches[0];

    if (view == SMS_LIST) {
        scroll.setContent((int)messages.size() * itemHeight, SCREEN_HEIGHT - 24);
        if (!scroll.handleTouch(tp) && tp.type == TAP) {
            int tapped = (tp.y - 24 + scroll.scrollY) / itemHeight;
            if (tapped >= 0 && tapped < (int)messages.size()) {
                selectedIdx = tapped;
                view = SMS_DETAIL;
                scroll.reset();
            }
        }
    } else if (view == SMS_DETAIL) {
        scroll.setContent(600, SCREEN_HEIGHT - 56);
        scroll.handleTouch(tp);
    }
}

void SmsNonStaticApp::CloseApp() {
    messages.clear();
    view = SMS_LIST;
    scroll.reset();
    selectedIdx = 0;
    loading = false;
}

const uint16_t* SmsNonStaticApp::getIcon() { return sms_icon; }
const uint16_t* SmsNonStaticApp::StaticIcon() { return sms_icon; }
