#pragma once
#include "appTemplates/nonStaticApp.h"
#include "System/systemGlobals.h"
#include "System/SystemUI/Scrollable.h"
#include <SD_MMC.h>
#include <vector>
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"

class MainMenu;
class AppMenu;

struct XmppMsg {
    String jid;
    String body;
    bool outgoing;
};

enum XmppView { XV_SETUP, XV_STATUS, XV_CONTACTS, XV_CHAT };

class XmppNonStaticApp : public NonStaticApp {
public:
    XmppNonStaticApp(const std::string& name);
    void Loop() override;
    void UpdateButtons(int button) override;
    void UpdateTouch(const TouchPoint* touches, int count) override;
    void Setup() override;
    void Draw() override;
    const uint16_t* getIcon() override;
    static const uint16_t* StaticIcon();
    void CloseApp() override;

private:
    TFT_eSprite *screenBuff = nullptr;
    String _server, _jid, _pass, _domain, _localpart;

    int _sock = -1;
    bool _tls = false;
    mbedtls_ssl_context _ssl;
    mbedtls_ssl_config _conf;
    mbedtls_ctr_drbg_context _ctr_drbg;
    mbedtls_entropy_context _entropy;
    String _recvBuf;

    TaskHandle_t _task = nullptr;
    SemaphoreHandle_t _mutex = nullptr;
    volatile bool _running = false;
    volatile int _progress = 0;
    String _error;

    std::vector<XmppMsg> _msgs;
    std::vector<String> _contacts;
    String _chatJid;
    int _lastChatCount = 0;

    String _outTo, _outBody;
    volatile bool _outPending = false;

    XmppView _view = XV_SETUP;
    Scrollable _scroll;
    int _setupField = 0;
    int _selContact = 0;

    void loadSettings();
    void saveSettings();
    void startConn();
    void stopConn();
    static void taskFunc(void* param);
    void connLoop();
    void netSend(const String& data);
    String netRecv();
    String readUntil(const char* marker, int timeoutMs);
    bool upgradeTls();
    void netClose();
    void parseStanza(const String& s);
    static String bare(const String& jid);

    void drawSetup();
    void drawStatus();
    void drawContacts();
    void drawChat();

    void loadChatFromSD(const String& jid);
    void appendMsgToSD(const String& jid, const String& body, bool outgoing);
    void saveContactsToSD();
    void loadContactsFromSD();
    void ensureXmppDirs();

    static const int ITEM_H = 36;
};
