#include "nonStaticPrograms/wifi_settings.h"
#include "staticPrograms/mainMenu.h"
#include "staticPrograms/appMenu.h"

static const uint16_t wifi_icon[256] = {0};
static const char* WIFI_DIR = "/system/wifi";

WifiSettingsApp::WifiSettingsApp(const std::string& name)
    : NonStaticApp(name) {}

void WifiSettingsApp::Setup() {
    screenBuff = &SystemDrivers::Get().GetScreenBuff();
    SD_MMC.mkdir("/system");
    SD_MMC.mkdir(WIFI_DIR);
    if (WiFi.status() == WL_CONNECTED) {
        connectSSID = WiFi.SSID();
        view = WV_CONNECTED;
    } else {
        view = WV_OFF;
    }
}

// ========== SD password storage ==========

static String sanitize(const String& ssid) {
    String s;
    for (unsigned int i = 0; i < ssid.length(); i++) {
        char c = ssid[i];
        if (isalnum(c) || c == '-' || c == '_') s += c;
        else s += '_';
    }
    return s;
}

String WifiSettingsApp::loadPassword(const String& ssid) {
    String path = String(WIFI_DIR) + "/" + sanitize(ssid) + ".txt";
    File f = SD_MMC.open(path, FILE_READ);
    if (!f) return "";
    String pass = f.readString();
    f.close();
    pass.trim();
    return pass;
}

void WifiSettingsApp::savePassword(const String& ssid, const String& pass) {
    String path = String(WIFI_DIR) + "/" + sanitize(ssid) + ".txt";
    File f = SD_MMC.open(path, FILE_WRITE);
    if (f) { f.print(pass); f.close(); }
}

// ========== WiFi on/off ==========

void WifiSettingsApp::wifiOn() {
    WiFi.mode(WIFI_STA);
    view = WV_SCANNING;
}

void WifiSettingsApp::wifiOff() {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    networks.clear();
    connectSSID = "";
    scroll.reset();
    selectedIdx = 0;
    view = WV_OFF;
}

// ========== Scan ==========

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
    view = WV_LIST;
}

// ========== Connect ==========

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
        view = WV_CONNECTING;
        return;
    }

    String saved = loadPassword(networks[idx].ssid);
    if (saved.length() > 0) {
        connectSSID = networks[idx].ssid;
        WiFi.begin(connectSSID.c_str(), saved.c_str());
        connectStart = millis();
        view = WV_CONNECTING;
    } else {
        String ssid = networks[idx].ssid;
        Keyboard::Get().Open("Password", [this, ssid](const String& pwd) {
            connectSSID = ssid;
            savePassword(ssid, pwd);
            WiFi.begin(connectSSID.c_str(), pwd.c_str());
            connectStart = millis();
            view = WV_CONNECTING;
        });
    }
}

// ========== Loop ==========

void WifiSettingsApp::Loop() {
    if (view == WV_SCANNING) {
        Draw();
        screenBuff->pushSprite(0, 0);
        scanNetworks();
        return;
    }
    if (view == WV_CONNECTING) {
        if (WiFi.status() == WL_CONNECTED) {
            view = WV_CONNECTED;
        } else if (millis() - connectStart > 15000) {
            WiFi.disconnect();
            view = WV_FAILED;
        }
    }
    Draw();
}

// ========== Drawing ==========

void WifiSettingsApp::Draw() {
    screenBuff->fillScreen(TFT_BLACK);
    switch (view) {
        case WV_OFF:        drawOff(); break;
        case WV_SCANNING:   drawScanning(); break;
        case WV_LIST:       drawList(); break;
        case WV_CONNECTING: drawConnecting(); break;
        case WV_CONNECTED:  drawConnected(); break;
        case WV_FAILED:     drawFailed(); break;
    }
}

void WifiSettingsApp::drawOff() {
    screenBuff->setTextSize(1);
    screenBuff->setTextDatum(MC_DATUM);
    screenBuff->setTextColor(TFT_DARKGREY);
    screenBuff->drawString("WiFi is OFF", 120, 130);
    screenBuff->fillRoundRect(80, 160, 80, 34, 6, 0x0320);
    screenBuff->setTextColor(TFT_WHITE);
    screenBuff->drawString("Turn ON", 120, 177);
    screenBuff->setTextDatum(TL_DATUM);
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

    // Turn OFF button top-right
    screenBuff->fillRoundRect(192, 24, 44, 16, 3, TFT_RED);
    screenBuff->setTextColor(TFT_WHITE);
    screenBuff->setTextDatum(MC_DATUM);
    screenBuff->drawString("OFF", 214, 32);
    screenBuff->setTextDatum(TL_DATUM);

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

        bool isConn = connected && networks[i].ssid == curSSID;

        if (isConn)
            screenBuff->fillRect(0, y, 240, ITEM_H - 2, 0x0320);
        else if (i == selectedIdx)
            screenBuff->fillRect(0, y, 240, ITEM_H - 2, 0x1082);

        drawSignalIcon(6, y + 8, networks[i].rssi);

        screenBuff->setTextColor(TFT_WHITE);
        String display = networks[i].ssid;
        if (display.length() > 22) display = display.substring(0, 22);
        screenBuff->drawString(display.c_str(), 24, y + 6);

        screenBuff->setTextColor(TFT_DARKGREY);
        String sub;
        if (isConn)
            sub = "Connected - tap to disconnect";
        else {
            sub = authStr(networks[i].encryption);
            if (networks[i].encryption != WIFI_AUTH_OPEN && loadPassword(networks[i].ssid).length() > 0)
                sub += " (saved)";
        }
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

// ========== Input ==========

void WifiSettingsApp::UpdateButtons(int button) {
    if (view == WV_SCANNING || view == WV_CONNECTING) return;

    if (button == BUTTON_BACK) {
        if (view == WV_LIST || view == WV_OFF)
            SystemCommon::Get().SetNextApp(&MainMenu::Get());
        else {
            view = WV_LIST;
            scroll.reset();
        }
        return;
    }
    if (button == BUTTON_HOME) {
        SystemCommon::Get().SetNextApp(&MainMenu::Get());
        return;
    }

    if (view == WV_OFF) {
        if (button == BUTTON_IN) wifiOn();
        return;
    }

    if (view == WV_LIST) {
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
        if (button == BUTTON_IN && !networks.empty())
            selectNetwork(selectedIdx);
        if (button == BUTTON_KEY1)
            view = WV_SCANNING;
        if (button == BUTTON_KEY2)
            wifiOff();
    }
    if (view == WV_FAILED && button == BUTTON_KEY1)
        view = WV_SCANNING;
}

void WifiSettingsApp::UpdateTouch(const TouchPoint* touches, int count) {
    if (count <= 0) return;
    const TouchPoint& tp = touches[0];

    if (view == WV_OFF && tp.type == TAP) {
        if (tp.x >= 80 && tp.x < 160 && tp.y >= 160 && tp.y < 194)
            wifiOn();
        return;
    }

    if (view == WV_LIST) {
        // OFF button
        if (tp.type == TAP && tp.x >= 192 && tp.y >= 24 && tp.y < 40) {
            wifiOff();
            return;
        }
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
    view = WV_OFF;
    scroll.reset();
    selectedIdx = 0;
}

const uint16_t* WifiSettingsApp::getIcon() { return wifi_icon; }
const uint16_t* WifiSettingsApp::StaticIcon() { return wifi_icon; }
