#pragma once
#include "appTemplates/nonStaticApp.h"
#include "System/systemGlobals.h"
#include "System/SystemUI/Scrollable.h"
#include "System/SystemUI/Keyboard.h"
#include <WiFi.h>
#include <SD_MMC.h>
#include <vector>

class MainMenu;
class AppMenu;

struct WifiNetwork {
    String ssid;
    int rssi;
    int encryption;
};

enum WifiView {
    WV_OFF,
    WV_SCANNING,
    WV_LIST,
    WV_CONNECTING,
    WV_CONNECTED,
    WV_FAILED
};

class WifiSettingsApp : public NonStaticApp {
public:
    WifiSettingsApp(const std::string& name);

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

    std::vector<WifiNetwork> networks;
    int selectedIdx = 0;
    Scrollable scroll;
    WifiView view = WV_OFF;

    String connectSSID;
    unsigned long connectStart = 0;

    void wifiOn();
    void wifiOff();
    void scanNetworks();
    void selectNetwork(int idx);
    String loadPassword(const String& ssid);
    void savePassword(const String& ssid, const String& pass);

    void drawOff();
    void drawScanning();
    void drawList();
    void drawConnecting();
    void drawConnected();
    void drawFailed();
    const char* authStr(int auth);
    void drawSignalIcon(int x, int y, int rssi);

    static const int ITEM_H = 36;
};
