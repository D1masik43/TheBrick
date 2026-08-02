#include "nonStaticPrograms/wifi_settings.h"
#include "staticPrograms/mainMenu.h"
#include "staticPrograms/appMenu.h"

static const uint16_t wifi_icon[256] = {0};

WifiSettingsApp::WifiSettingsApp(const std::string& name)
    : NonStaticApp(name) {}

void WifiSettingsApp::Setup() {
    screenBuff = &SystemDrivers::Get().GetScreenBuff();
    view = WIFI_SCANNING;
    if (WiFi.status() != WL_CONNECTED)
        WiFi.mode(WIFI_STA);
    else
        connectSSID = WiFi.SSID();
}

void WifiSettingsApp::scanNetworks() {
    networks.clear();
    selectedIdx = 0;
    scroll.reset();

    int n = WiFi.scanNetworks();
    for (int i = 0; i < n && i < 30; i++) {
        WifiNetwork net;
        net.ssid = WiFi.SSID(i);
        net.rssi = WiFi.RSSI(i);
        net.encryption = WiFi.encryptionType(i);
        if (net.ssid.length() == 0) continue;
        bool dup = false;
        for (auto& existing : networks) {
            if (existing.ssid == net.ssid) { dup = true; break; }
        }
        if (!dup) networks.push_back(net);
    }
    WiFi.scanDelete();
    view = WIFI_LIST;
}

void WifiSettingsApp::Loop() {
    if (view == WIFI_SCANNING) {
        Draw();
        screenBuff->pushSprite(0, 0);
        scanNetworks();
        return;
    }
    if (view == WIFI_CONNECTING) {
        if (WiFi.status() == WL_CONNECTED) {
            view = WIFI_CONNECTED;
        } else if (millis() - connectStart > 15000) {
            WiFi.disconnect();
            view = WIFI_FAILED;
        }
    }
    Draw();
}

void WifiSettingsApp::Draw() {
    screenBuff->fillScreen(TFT_BLACK);
    switch (view) {
        case WIFI_SCANNING:   drawScanning(); break;
        case WIFI_LIST:       drawList(); break;
        case WIFI_CONNECTING: drawConnecting(); break;
        case WIFI_CONNECTED:  drawConnected(); break;
        case WIFI_FAILED:     drawFailed(); break;
    }
}

void WifiSettingsApp::drawScanning() {
    screenBuff->setTextSize(1);
    screenBuff->setTextDatum(MC_DATUM);
    screenBuff->setTextColor(TFT_DARKGREY);
    screenBuff->drawString("Scanning...", 120, 160);
    screenBuff->setTextDatum(TL_DATUM);
}

void WifiSettingsApp::drawList() {
    screenBuff->setTextSize(1);
    screenBuff->setTextDatum(TL_DATUM);

    screenBuff->setTextColor(TFT_WHITE);
    screenBuff->drawString("WiFi Networks", 8, 26);

    if (WiFi.status() == WL_CONNECTED) {
        screenBuff->setTextColor(TFT_GREEN);
        String info = "Connected: " + WiFi.SSID();
        screenBuff->drawString(info.c_str(), 8, 38);
    }

    int listTop = 52;

    if (networks.empty()) {
        screenBuff->setTextColor(TFT_DARKGREY);
        screenBuff->drawString("No networks found", 60, 160);
        screenBuff->drawString("[KEY1] Rescan", 70, 180);
        return;
    }

    bool connected = WiFi.status() == WL_CONNECTED;
    String curSSID = connected ? WiFi.SSID() : "";

    for (int i = 0; i < (int)networks.size(); i++) {
        int y = listTop + i * ITEM_H - scroll.scrollY;
        if (y + ITEM_H < listTop) continue;
        if (y > SCREEN_HEIGHT) break;

        bool isConnected = connected && networks[i].ssid == curSSID;

        if (isConnected)
            screenBuff->fillRect(0, y, 240, ITEM_H - 2, 0x0320);
        else if (i == selectedIdx)
            screenBuff->fillRect(0, y, 240, ITEM_H - 2, 0x1082);

        drawSignalIcon(6, y + 8, networks[i].rssi);

        screenBuff->setTextColor(isConnected ? TFT_WHITE : TFT_WHITE);
        String display = networks[i].ssid;
        if (display.length() > 22) display = display.substring(0, 22);
        screenBuff->drawString(display.c_str(), 24, y + 6);

        screenBuff->setTextColor(TFT_DARKGREY);
        String sub = isConnected ? "Connected - tap to disconnect" : String(authStr(networks[i].encryption));
        screenBuff->drawString(sub.c_str(), 24, y + 18);

        screenBuff->drawLine(0, y + ITEM_H - 2, 240, y + ITEM_H - 2, 0x2104);
    }
}

void WifiSettingsApp::drawConnecting() {
    screenBuff->setTextSize(1);
    screenBuff->setTextDatum(MC_DATUM);
    screenBuff->setTextColor(TFT_WHITE);
    screenBuff->drawString("Connecting to", 120, 140);
    screenBuff->setTextColor(TFT_CYAN);
    screenBuff->drawString(connectSSID.c_str(), 120, 156);

    int elapsed = (millis() - connectStart) / 1000;
    char buf[16];
    snprintf(buf, sizeof(buf), "%ds / 15s", elapsed);
    screenBuff->setTextColor(TFT_DARKGREY);
    screenBuff->drawString(buf, 120, 180);
    screenBuff->setTextDatum(TL_DATUM);
}

void WifiSettingsApp::drawConnected() {
    screenBuff->setTextSize(1);
    screenBuff->setTextDatum(MC_DATUM);

    screenBuff->setTextColor(TFT_GREEN);
    screenBuff->drawString("Connected!", 120, 120);

    screenBuff->setTextColor(TFT_WHITE);
    screenBuff->drawString(connectSSID.c_str(), 120, 140);

    screenBuff->setTextColor(TFT_CYAN);
    screenBuff->drawString(WiFi.localIP().toString().c_str(), 120, 160);

    screenBuff->setTextColor(TFT_DARKGREY);
    screenBuff->drawString("[BACK] Return", 120, 200);
    screenBuff->setTextDatum(TL_DATUM);
}

void WifiSettingsApp::drawFailed() {
    screenBuff->setTextSize(1);
    screenBuff->setTextDatum(MC_DATUM);

    screenBuff->setTextColor(TFT_RED);
    screenBuff->drawString("Connection failed", 120, 140);

    screenBuff->setTextColor(TFT_WHITE);
    screenBuff->drawString(connectSSID.c_str(), 120, 156);

    screenBuff->setTextColor(TFT_DARKGREY);
    screenBuff->drawString("[KEY1] Retry  [BACK] Back", 120, 190);
    screenBuff->setTextDatum(TL_DATUM);
}

void WifiSettingsApp::drawSignalIcon(int x, int y, int rssi) {
    int bars = 0;
    if (rssi > -50) bars = 4;
    else if (rssi > -60) bars = 3;
    else if (rssi > -70) bars = 2;
    else if (rssi > -80) bars = 1;

    for (int i = 0; i < 4; i++) {
        int bh = 4 + i * 4;
        int by = y + 16 - bh;
        uint16_t color = (i < bars) ? TFT_WHITE : 0x2104;
        screenBuff->fillRect(x + i * 4, by, 3, bh, color);
    }
}

const char* WifiSettingsApp::authStr(int auth) {
    switch (auth) {
        case WIFI_AUTH_OPEN: return "Open";
        case WIFI_AUTH_WEP: return "WEP";
        case WIFI_AUTH_WPA_PSK: return "WPA";
        case WIFI_AUTH_WPA2_PSK: return "WPA2";
        case WIFI_AUTH_WPA_WPA2_PSK: return "WPA/WPA2";
        default: return "Secured";
    }
}

void WifiSettingsApp::selectNetwork(int idx) {
    if (idx < 0 || idx >= (int)networks.size()) return;

    if (WiFi.status() == WL_CONNECTED && networks[idx].ssid == WiFi.SSID()) {
        WiFi.disconnect();
        connectSSID = "";
        return;
    }

    if (networks[idx].encryption == WIFI_AUTH_OPEN) {
        connectSSID = networks[idx].ssid;
        WiFi.begin(connectSSID.c_str());
        connectStart = millis();
        view = WIFI_CONNECTING;
    } else {
        String ssid = networks[idx].ssid;
        Keyboard::Get().Open("Password", [this, ssid](const String& pwd) {
            connectSSID = ssid;
            WiFi.begin(connectSSID.c_str(), pwd.c_str());
            connectStart = millis();
            view = WIFI_CONNECTING;
        });
    }
}

void WifiSettingsApp::UpdateButtons(int button) {
    if (view == WIFI_SCANNING || view == WIFI_CONNECTING) return;

    if (button == BUTTON_BACK) {
        if (view == WIFI_LIST) {
            SystemCommon::Get().SetNextApp(&MainMenu::Get());
        } else {
            view = WIFI_LIST;
            scroll.reset();
        }
        return;
    }
    if (button == BUTTON_HOME) {
        SystemCommon::Get().SetNextApp(&MainMenu::Get());
        return;
    }

    if (view == WIFI_LIST) {
        if (button == BUTTON_UP && selectedIdx > 0) {
            selectedIdx--;
            int topY = selectedIdx * ITEM_H - scroll.scrollY;
            if (topY < 0) scroll.scrollY = selectedIdx * ITEM_H;
        }
        if (button == BUTTON_DOWN && selectedIdx < (int)networks.size() - 1) {
            selectedIdx++;
            int botY = selectedIdx * ITEM_H - scroll.scrollY + ITEM_H;
            if (botY > SCREEN_HEIGHT - 52)
                scroll.scrollY = selectedIdx * ITEM_H + ITEM_H - (SCREEN_HEIGHT - 52);
        }
        if (button == BUTTON_IN && !networks.empty()) {
            selectNetwork(selectedIdx);
        }
        if (button == BUTTON_KEY1) {
            view = WIFI_SCANNING;
        }
    }
    if (view == WIFI_FAILED && button == BUTTON_KEY1) {
        view = WIFI_SCANNING;
    }
}

void WifiSettingsApp::UpdateTouch(const TouchPoint* touches, int count) {
    if (count <= 0) return;
    const TouchPoint& tp = touches[0];

    if (view == WIFI_LIST) {
        scroll.setContent((int)networks.size() * ITEM_H, SCREEN_HEIGHT - 52);
        if (!scroll.handleTouch(tp) && tp.type == TAP) {
            int tapped = (tp.y - 52 + scroll.scrollY) / ITEM_H;
            if (tapped >= 0 && tapped < (int)networks.size()) {
                selectedIdx = tapped;
                selectNetwork(tapped);
            }
        }
    }
}

void WifiSettingsApp::CloseApp() {
    networks.clear();
    view = WIFI_SCANNING;
    scroll.reset();
    selectedIdx = 0;
}

const uint16_t* WifiSettingsApp::getIcon() { return wifi_icon; }
const uint16_t* WifiSettingsApp::StaticIcon() { return wifi_icon; }
