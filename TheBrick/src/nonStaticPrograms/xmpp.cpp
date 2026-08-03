#include "nonStaticPrograms/xmpp.h"
#include "staticPrograms/mainMenu.h"
#include "staticPrograms/appMenu.h"
#include "System/SystemUI/Keyboard.h"
#include <WiFi.h>
#include <Preferences.h>
#include <lwip/sockets.h>
#include <errno.h>
#include "mbedtls/base64.h"
#include "mbedtls/error.h"

static const uint16_t xmpp_icon[256] = {0};

static String xmlEnc(const String& s) {
    String r = s;
    r.replace("&", "&amp;");
    r.replace("<", "&lt;");
    r.replace(">", "&gt;");
    return r;
}

static String xmlDec(const String& s) {
    String r = s;
    r.replace("&amp;", "&");
    r.replace("&lt;", "<");
    r.replace("&gt;", ">");
    r.replace("&quot;", "\"");
    r.replace("&apos;", "'");
    return r;
}

static int tls_send_cb(void* ctx, const unsigned char* buf, size_t len) {
    int fd = *(int*)ctx;
    int ret = lwip_send(fd, buf, len, 0);
    return (ret < 0) ? MBEDTLS_ERR_NET_SEND_FAILED : ret;
}

static int tls_recv_cb(void* ctx, unsigned char* buf, size_t len) {
    int fd = *(int*)ctx;
    int ret = lwip_recv(fd, buf, len, 0);
    if (ret < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return MBEDTLS_ERR_SSL_WANT_READ;
        return MBEDTLS_ERR_NET_RECV_FAILED;
    }
    if (ret == 0) return MBEDTLS_ERR_NET_CONN_RESET;
    return ret;
}

// ========== Constructor ==========

XmppNonStaticApp::XmppNonStaticApp(const std::string& name)
    : NonStaticApp(name) {}

// ========== SD Persistence ==========

static String sanitizeJid(const String& jid) {
    String s;
    for (unsigned int i = 0; i < jid.length(); i++) {
        char c = jid[i];
        if (isalnum(c) || c == '-' || c == '_' || c == '.') s += c;
        else s += '_';
    }
    return s;
}

void XmppNonStaticApp::ensureXmppDirs() {
    SD_MMC.mkdir("/system");
    SD_MMC.mkdir("/system/xmpp");
}

void XmppNonStaticApp::saveContactsToSD() {
    ensureXmppDirs();
    File f = SD_MMC.open("/system/xmpp/contacts.txt", FILE_WRITE);
    if (!f) return;
    for (auto& c : _contacts) {
        f.println(c);
    }
    f.close();
}

void XmppNonStaticApp::loadContactsFromSD() {
    File f = SD_MMC.open("/system/xmpp/contacts.txt", FILE_READ);
    if (!f) return;
    while (f.available()) {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line.length() == 0) continue;
        bool found = false;
        for (auto& c : _contacts) { if (c == line) { found = true; break; } }
        if (!found) _contacts.push_back(line);
    }
    f.close();
}

void XmppNonStaticApp::loadChatFromSD(const String& jid) {
    ensureXmppDirs();
    String path = "/system/xmpp/" + sanitizeJid(jid) + ".txt";
    File f = SD_MMC.open(path, FILE_READ);
    if (!f) return;
    while (f.available()) {
        String line = f.readStringUntil('\n');
        if (line.length() < 2) continue;
        bool out = (line[0] == '>');
        String body = line.substring(1);
        body.trim();
        if (body.length() > 0)
            _msgs.push_back({jid, body, out});
    }
    f.close();
    if (_msgs.size() > 200) {
        _msgs.erase(_msgs.begin(), _msgs.begin() + (_msgs.size() - 200));
    }
}

void XmppNonStaticApp::appendMsgToSD(const String& jid, const String& body, bool outgoing) {
    ensureXmppDirs();
    String path = "/system/xmpp/" + sanitizeJid(jid) + ".txt";
    File f = SD_MMC.open(path, FILE_APPEND);
    if (!f) return;
    f.print(outgoing ? ">" : "<");
    f.println(body);
    f.close();
}

// ========== Settings ==========

void XmppNonStaticApp::loadSettings() {
    Preferences prefs;
    prefs.begin("xmpp", true);
    _server = prefs.getString("server", "");
    _jid = prefs.getString("jid", "");
    _pass = prefs.getString("pass", "");
    prefs.end();
    int at = _jid.indexOf('@');
    if (at > 0) {
        _localpart = _jid.substring(0, at);
        _domain = _jid.substring(at + 1);
    }
}

void XmppNonStaticApp::saveSettings() {
    Preferences prefs;
    prefs.begin("xmpp", false);
    prefs.putString("server", _server);
    prefs.putString("jid", _jid);
    prefs.putString("pass", _pass);
    prefs.end();
    int at = _jid.indexOf('@');
    if (at > 0) {
        _localpart = _jid.substring(0, at);
        _domain = _jid.substring(at + 1);
    }
}

// ========== Network I/O ==========

void XmppNonStaticApp::netSend(const String& data) {
    if (_sock < 0) return;
    const char* p = data.c_str();
    int rem = data.length();
    while (rem > 0) {
        int ret;
        if (_tls)
            ret = mbedtls_ssl_write(&_ssl, (const unsigned char*)p, rem);
        else
            ret = lwip_send(_sock, p, rem, 0);
        if (ret <= 0) return;
        p += ret;
        rem -= ret;
    }
}

String XmppNonStaticApp::netRecv() {
    char buf[1024];
    int ret;
    if (_tls) {
        ret = mbedtls_ssl_read(&_ssl, (unsigned char*)buf, sizeof(buf) - 1);
        if (ret == MBEDTLS_ERR_SSL_WANT_READ) return "";
    } else {
        ret = lwip_recv(_sock, buf, sizeof(buf) - 1, 0);
        if (ret < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) return "";
    }
    if (ret <= 0) return "";
    buf[ret] = 0;
    return String(buf);
}

String XmppNonStaticApp::readUntil(const char* marker, int timeoutMs) {
    unsigned long start = millis();
    while (_running && (millis() - start < (unsigned long)timeoutMs)) {
        String chunk = netRecv();
        if (chunk.length() > 0) _recvBuf += chunk;
        int idx = _recvBuf.indexOf(marker);
        if (idx >= 0) {
            int end = idx + strlen(marker);
            String result = _recvBuf.substring(0, end);
            _recvBuf = _recvBuf.substring(end);
            return result;
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    return "";
}

bool XmppNonStaticApp::upgradeTls() {
    mbedtls_ssl_init(&_ssl);
    mbedtls_ssl_config_init(&_conf);
    mbedtls_ctr_drbg_init(&_ctr_drbg);
    mbedtls_entropy_init(&_entropy);

    if (mbedtls_ctr_drbg_seed(&_ctr_drbg, mbedtls_entropy_func, &_entropy, NULL, 0) != 0)
        goto fail;
    if (mbedtls_ssl_config_defaults(&_conf, MBEDTLS_SSL_IS_CLIENT,
            MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT) != 0)
        goto fail;

    mbedtls_ssl_conf_authmode(&_conf, MBEDTLS_SSL_VERIFY_NONE);
    mbedtls_ssl_conf_rng(&_conf, mbedtls_ctr_drbg_random, &_ctr_drbg);

    if (mbedtls_ssl_setup(&_ssl, &_conf) != 0) goto fail;
    mbedtls_ssl_set_hostname(&_ssl, _domain.c_str());
    mbedtls_ssl_set_bio(&_ssl, &_sock, tls_send_cb, tls_recv_cb, NULL);

    {
        struct timeval tv = {10, 0};
        lwip_setsockopt(_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        int ret = mbedtls_ssl_handshake(&_ssl);

        tv = {0, 200000};
        lwip_setsockopt(_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        if (ret != 0) {
            char err[100];
            mbedtls_strerror(ret, err, sizeof(err));
            _error = String("TLS: ") + err;
            goto fail;
        }
    }

    _tls = true;
    return true;

fail:
    mbedtls_ssl_free(&_ssl);
    mbedtls_ssl_config_free(&_conf);
    mbedtls_ctr_drbg_free(&_ctr_drbg);
    mbedtls_entropy_free(&_entropy);
    return false;
}

void XmppNonStaticApp::netClose() {
    if (_tls) {
        mbedtls_ssl_close_notify(&_ssl);
        mbedtls_ssl_free(&_ssl);
        mbedtls_ssl_config_free(&_conf);
        mbedtls_ctr_drbg_free(&_ctr_drbg);
        mbedtls_entropy_free(&_entropy);
        _tls = false;
    }
    if (_sock >= 0) {
        lwip_close(_sock);
        _sock = -1;
    }
}

// ========== XMPP Protocol ==========

String XmppNonStaticApp::bare(const String& jid) {
    int slash = jid.indexOf('/');
    return (slash > 0) ? jid.substring(0, slash) : jid;
}

void XmppNonStaticApp::parseStanza(const String& s) {
    if (s.indexOf("<message") < 0) return;
    if (s.indexOf("<body>") < 0 && s.indexOf("encrypted") < 0 &&
        s.indexOf("omemo") < 0 && s.indexOf("axolotl") < 0) {
        Serial.println("[XMPP] skip: no body, not encrypted");
        return;
    }
    if (s.indexOf("type=\"error\"") >= 0 || s.indexOf("type='error'") >= 0) return;

    // extract from= with either quote type
    String from;
    int fromS = s.indexOf("from=\"");
    if (fromS >= 0) {
        fromS += 6;
        int fromE = s.indexOf("\"", fromS);
        if (fromE > fromS) from = s.substring(fromS, fromE);
    } else {
        fromS = s.indexOf("from='");
        if (fromS >= 0) {
            fromS += 6;
            int fromE = s.indexOf("'", fromS);
            if (fromE > fromS) from = s.substring(fromS, fromE);
        }
    }
    if (from.length() == 0) {
        Serial.println("[XMPP] skip: no from attr");
        return;
    }
    from = bare(from);
    if (from == _jid) return;

    String body;
    int bodyS = s.indexOf("<body>");
    if (bodyS >= 0) {
        bodyS += 6;
        int bodyE = s.indexOf("</body>", bodyS);
        if (bodyE >= 0)
            body = xmlDec(s.substring(bodyS, bodyE));
    }

    if (body.length() == 0) {
        if (s.indexOf("encrypted") >= 0 || s.indexOf("omemo") >= 0 ||
            s.indexOf("axolotl") >= 0 || s.indexOf("openpgp") >= 0)
            body = "[encrypted]";
        else {
            Serial.println("[XMPP] skip: body empty after parse");
            return;
        }
    }

    Serial.printf("[XMPP] from=%s body=%s\n", from.c_str(), body.c_str());

    xSemaphoreTake(_mutex, portMAX_DELAY);
    _msgs.push_back({from, body, false});
    if (_msgs.size() > 200) _msgs.erase(_msgs.begin());
    bool found = false;
    for (auto& c : _contacts) { if (c == from) { found = true; break; } }
    if (!found) _contacts.push_back(from);
    xSemaphoreGive(_mutex);

    appendMsgToSD(from, body, false);
    if (!found) saveContactsToSD();
}

// ========== Connection Task ==========

void XmppNonStaticApp::taskFunc(void* param) {
    XmppNonStaticApp* self = (XmppNonStaticApp*)param;
    self->connLoop();
    self->_running = false;
    self->_task = nullptr;
    vTaskDelete(NULL);
}

void XmppNonStaticApp::connLoop() {
    // TCP connect
    _progress = 1;
    IPAddress ip;
    if (!WiFi.hostByName(_server.c_str(), ip)) {
        _error = "DNS failed"; _progress = -1; return;
    }
    _sock = lwip_socket(AF_INET, SOCK_STREAM, 0);
    if (_sock < 0) {
        _error = "Socket failed"; _progress = -1; return;
    }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(5222);
    addr.sin_addr.s_addr = ip;

    struct timeval tv = {10, 0};
    lwip_setsockopt(_sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    lwip_setsockopt(_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    if (lwip_connect(_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        _error = "Connect failed"; _progress = -1;
        lwip_close(_sock); _sock = -1; return;
    }

    tv = {0, 200000};
    lwip_setsockopt(_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    String streamOpen = "<?xml version='1.0'?><stream:stream to='" + _domain +
        "' xmlns='jabber:client' xmlns:stream='http://etherx.jabber.org/streams' version='1.0'>";

    // Stream 1
    _progress = 2;
    netSend(streamOpen);
    String features = readUntil("</stream:features>", 10000);
    if (features.length() == 0) {
        _error = "No response"; _progress = -1; netClose(); return;
    }

    // STARTTLS
    if (features.indexOf("starttls") >= 0) {
        _progress = 3;
        netSend("<starttls xmlns='urn:ietf:params:xml:ns:xmpp-tls'/>");
        String resp = readUntil(">", 10000);
        if (resp.indexOf("proceed") < 0) {
            _error = "STARTTLS rejected"; _progress = -1; netClose(); return;
        }
        if (!upgradeTls()) {
            if (_error.length() == 0) _error = "TLS failed";
            _progress = -1; netClose(); return;
        }
        _recvBuf = "";
        netSend(streamOpen);
        features = readUntil("</stream:features>", 10000);
        if (features.length() == 0) {
            _error = "Post-TLS failed"; _progress = -1; netClose(); return;
        }
    }

    // SASL PLAIN auth
    _progress = 4;
    {
        uint8_t plain[256];
        int plen = 0;
        plain[plen++] = 0;
        memcpy(plain + plen, _localpart.c_str(), _localpart.length());
        plen += _localpart.length();
        plain[plen++] = 0;
        memcpy(plain + plen, _pass.c_str(), _pass.length());
        plen += _pass.length();

        unsigned char b64[512];
        size_t b64len;
        mbedtls_base64_encode(b64, sizeof(b64), &b64len, plain, plen);
        b64[b64len] = 0;

        netSend(String("<auth xmlns='urn:ietf:params:xml:ns:xmpp-sasl' mechanism='PLAIN'>") +
                (char*)b64 + "</auth>");
    }

    String authResp = readUntil(">", 10000);
    if (authResp.indexOf("success") < 0) {
        _error = "Auth failed"; _progress = -1; netClose(); return;
    }

    // Stream restart after auth
    _progress = 5;
    _recvBuf = "";
    netSend(streamOpen);
    features = readUntil("</stream:features>", 10000);

    // Bind resource
    netSend("<iq type='set' id='b1'><bind xmlns='urn:ietf:params:xml:ns:xmpp-bind'>"
            "<resource>TheBrick</resource></bind></iq>");
    readUntil("</iq>", 10000);

    // Session
    _progress = 6;
    netSend("<iq type='set' id='s1'><session xmlns='urn:ietf:params:xml:ns:xmpp-session'/></iq>");
    readUntil("</iq>", 10000);

    // Send presence
    netSend("<presence/>");
    _progress = 7;

    // Main loop
    while (_running) {
        String data = netRecv();
        if (data.length() > 0) {
            _recvBuf += data;

            // strip presence and iq first so they don't contaminate message extraction
            int end;
            while ((end = _recvBuf.indexOf("</presence>")) >= 0)
                _recvBuf = _recvBuf.substring(end + 11);
            while ((end = _recvBuf.indexOf("</iq>")) >= 0)
                _recvBuf = _recvBuf.substring(end + 5);

            while ((end = _recvBuf.indexOf("</message>")) >= 0) {
                end += 10;
                int start = _recvBuf.lastIndexOf("<message", end);
                if (start >= 0) {
                    String stanza = _recvBuf.substring(start, end);
                    Serial.printf("[XMPP-MSG] %s\n", stanza.c_str());
                    parseStanza(stanza);
                }
                _recvBuf = _recvBuf.substring(end);
            }
            if ((int)_recvBuf.length() > 8192)
                _recvBuf = "";
        }

        if (_outPending) {
            String msg = "<message to='" + xmlEnc(_outTo) + "' type='chat' id='m" +
                         String(millis()) + "'><body>" + xmlEnc(_outBody) + "</body></message>";
            netSend(msg);
            String bareJid = bare(_outTo);
            xSemaphoreTake(_mutex, portMAX_DELAY);
            _msgs.push_back({bareJid, _outBody, true});
            if (_msgs.size() > 200) _msgs.erase(_msgs.begin());
            bool found = false;
            for (auto& c : _contacts) { if (c == bareJid) { found = true; break; } }
            if (!found) _contacts.push_back(bareJid);
            xSemaphoreGive(_mutex);
            appendMsgToSD(bareJid, _outBody, true);
            if (!found) saveContactsToSD();
            _outPending = false;
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }

    netSend("</stream:stream>");
    vTaskDelay(pdMS_TO_TICKS(100));
    netClose();
    _progress = 0;
}

void XmppNonStaticApp::startConn() {
    if (_task) return;
    _running = true;
    _progress = 0;
    _error = "";
    _recvBuf = "";
    xTaskCreate(taskFunc, "xmpp", 12288, this, 1, &_task);
}

void XmppNonStaticApp::stopConn() {
    _running = false;
    if (_task) {
        int tries = 100;
        while (_task && tries-- > 0) vTaskDelay(pdMS_TO_TICKS(50));
        if (_task) {
            vTaskDelete(_task);
            _task = nullptr;
            netClose();
        }
    }
    _progress = 0;
}

// ========== App Lifecycle ==========

void XmppNonStaticApp::Setup() {
    screenBuff = &SystemDrivers::Get().GetScreenBuff();
    _mutex = xSemaphoreCreateMutex();
    loadSettings();
    ensureXmppDirs();
    loadContactsFromSD();
    if (_server.length() > 0 && _jid.length() > 0 && _pass.length() > 0) {
        if (!_contacts.empty()) {
            _view = XV_CONTACTS;
            startConn();
        } else {
            _view = XV_STATUS;
            startConn();
        }
    } else {
        _view = XV_SETUP;
    }
}

void XmppNonStaticApp::Loop() {
    if (_view == XV_STATUS && _progress == 7)
        _view = XV_CONTACTS;

    if (_view == XV_CHAT) {
        xSemaphoreTake(_mutex, portMAX_DELAY);
        int count = 0;
        for (auto& m : _msgs) if (m.jid == _chatJid) count++;
        xSemaphoreGive(_mutex);
        if (count != _lastChatCount) {
            _lastChatCount = count;
            int totalH = count * 14;
            int viewH = SCREEN_HEIGHT - 60;
            if (totalH > viewH) _scroll.scrollY = totalH - viewH;
        }
    }

    Draw();
}

void XmppNonStaticApp::CloseApp() {
    stopConn();
    _msgs.clear();
    _contacts.clear();
    _recvBuf = "";
    _chatJid = "";
    _view = XV_SETUP;
    _setupField = 0;
    _selContact = 0;
    _lastChatCount = 0;
    _outPending = false;
    _scroll.reset();
    if (_mutex) { vSemaphoreDelete(_mutex); _mutex = nullptr; }
}

// ========== Drawing ==========

void XmppNonStaticApp::Draw() {
    screenBuff->fillScreen(TFT_BLACK);
    switch (_view) {
        case XV_SETUP:    drawSetup(); break;
        case XV_STATUS:   drawStatus(); break;
        case XV_CONTACTS: drawContacts(); break;
        case XV_CHAT:     drawChat(); break;
    }
}

void XmppNonStaticApp::drawSetup() {
    screenBuff->setTextSize(1);
    screenBuff->setTextDatum(TL_DATUM);
    screenBuff->setTextColor(TFT_WHITE);
    screenBuff->drawString("XMPP Setup", 10, 26);

    const char* labels[] = {"Server:", "JID:", "Password:"};
    String values[] = {_server, _jid, String()};
    for (int i = 0; i < (int)_pass.length() && i < 20; i++) values[2] += '*';
    if (_pass.length() == 0) values[2] = "";

    for (int i = 0; i < 3; i++) {
        int y = 56 + i * 46;
        if (i == _setupField)
            screenBuff->fillRect(0, y - 2, 240, 42, 0x1082);
        screenBuff->setTextColor(TFT_DARKGREY);
        screenBuff->drawString(labels[i], 10, y);
        screenBuff->setTextColor(TFT_WHITE);
        String val = values[i].length() > 0 ? values[i] : "(tap to set)";
        if (val.length() > 26) val = val.substring(0, 26);
        screenBuff->drawString(val.c_str(), 10, y + 14);
    }

    bool ok = _server.length() > 0 && _jid.length() > 0 && _pass.length() > 0;
    screenBuff->fillRoundRect(60, 210, 120, 30, 4, ok ? 0x0320 : 0x2104);
    screenBuff->setTextColor(TFT_WHITE);
    screenBuff->setTextDatum(MC_DATUM);
    screenBuff->drawString("Connect", 120, 225);
    screenBuff->setTextDatum(TL_DATUM);
}

void XmppNonStaticApp::drawStatus() {
    screenBuff->setTextSize(1);
    screenBuff->setTextDatum(MC_DATUM);

    const char* steps[] = {
        "", "TCP Connect...", "Stream...", "STARTTLS...",
        "Auth...", "Binding...", "Session...", "Connected!"
    };

    if (_progress > 0 && _progress <= 7) {
        screenBuff->setTextColor(_progress == 7 ? TFT_GREEN : TFT_WHITE);
        screenBuff->drawString(steps[_progress], 120, 140);
        if (_progress < 7) {
            char buf[8];
            snprintf(buf, sizeof(buf), "%d/7", _progress);
            screenBuff->setTextColor(TFT_DARKGREY);
            screenBuff->drawString(buf, 120, 160);
        }
    } else if (_progress == -1) {
        screenBuff->setTextColor(TFT_RED);
        screenBuff->drawString("Error", 120, 120);
        screenBuff->setTextColor(TFT_WHITE);
        String err = _error;
        if (err.length() > 30) err = err.substring(0, 30);
        screenBuff->drawString(err.c_str(), 120, 140);
        screenBuff->setTextColor(TFT_DARKGREY);
        screenBuff->drawString("[BACK] Return", 120, 180);
    }
    screenBuff->setTextDatum(TL_DATUM);
}

void XmppNonStaticApp::drawContacts() {
    screenBuff->setTextSize(1);
    screenBuff->setTextDatum(TL_DATUM);

    screenBuff->setTextColor(_progress == 7 ? TFT_GREEN : TFT_RED);
    screenBuff->drawString(_progress == 7 ? "XMPP Online" : "Offline", 8, 26);
    screenBuff->setTextColor(TFT_DARKGREY);
    screenBuff->drawString("[KEY1] New chat", 130, 26);

    int listTop = 44;

    xSemaphoreTake(_mutex, portMAX_DELAY);
    std::vector<String> contacts = _contacts;
    std::vector<String> lastMsgs;
    for (auto& c : contacts) {
        String last;
        for (int j = _msgs.size() - 1; j >= 0; j--) {
            if (_msgs[j].jid == c) {
                last = (_msgs[j].outgoing ? "> " : "") + _msgs[j].body;
                break;
            }
        }
        lastMsgs.push_back(last);
    }
    xSemaphoreGive(_mutex);

    int n = contacts.size();
    if (n == 0) {
        screenBuff->setTextColor(TFT_DARKGREY);
        screenBuff->setTextDatum(MC_DATUM);
        screenBuff->drawString("No conversations", 120, 160);
        screenBuff->drawString("[KEY1] Start new chat", 120, 180);
        screenBuff->setTextDatum(TL_DATUM);
        return;
    }

    for (int i = 0; i < n; i++) {
        int y = listTop + i * ITEM_H - _scroll.scrollY;
        if (y + ITEM_H < listTop) continue;
        if (y > SCREEN_HEIGHT) break;

        if (i == _selContact)
            screenBuff->fillRect(0, y, 240, ITEM_H - 2, 0x1082);

        screenBuff->setTextColor(TFT_CYAN);
        String disp = contacts[i];
        if (disp.length() > 28) disp = disp.substring(0, 28);
        screenBuff->drawString(disp.c_str(), 8, y + 4);

        String last = lastMsgs[i];
        if (last.length() > 30) last = last.substring(0, 30);
        screenBuff->setTextColor(TFT_DARKGREY);
        screenBuff->drawString(last.c_str(), 8, y + 16);

        screenBuff->drawLine(0, y + ITEM_H - 2, 240, y + ITEM_H - 2, 0x2104);
    }
}

void XmppNonStaticApp::drawChat() {
    screenBuff->setTextSize(1);
    screenBuff->setTextDatum(TL_DATUM);

    screenBuff->setTextColor(TFT_CYAN);
    String disp = _chatJid;
    if (disp.length() > 28) disp = disp.substring(0, 28);
    screenBuff->drawString(disp.c_str(), 8, 26);
    screenBuff->drawLine(0, 38, 240, 38, 0x2104);

    int listTop = 40;
    int bottomBar = 20;

    xSemaphoreTake(_mutex, portMAX_DELAY);
    std::vector<XmppMsg> chat;
    for (auto& m : _msgs)
        if (m.jid == _chatJid) chat.push_back(m);
    xSemaphoreGive(_mutex);

    int lineH = 14;
    for (int i = 0; i < (int)chat.size(); i++) {
        int y = listTop + i * lineH - _scroll.scrollY;
        if (y + lineH < listTop) continue;
        if (y > SCREEN_HEIGHT - bottomBar) break;

        screenBuff->setTextColor(chat[i].outgoing ? TFT_GREEN : TFT_WHITE);
        String prefix = chat[i].outgoing ? "> " : "< ";
        String line = prefix + chat[i].body;
        if (line.length() > 36) line = line.substring(0, 36);
        screenBuff->drawString(line.c_str(), 4, y);
    }

    screenBuff->fillRect(0, SCREEN_HEIGHT - bottomBar, 240, bottomBar, 0x0841);
    screenBuff->setTextColor(TFT_DARKGREY);
    screenBuff->setTextDatum(MC_DATUM);
    screenBuff->drawString("[IN] Send message", 120, SCREEN_HEIGHT - bottomBar / 2);
    screenBuff->setTextDatum(TL_DATUM);
}

// ========== Input ==========

void XmppNonStaticApp::UpdateButtons(int button) {
    if (button == BUTTON_HOME) {
        SystemCommon::Get().SetNextApp(&MainMenu::Get());
        return;
    }

    switch (_view) {
    case XV_SETUP:
        if (button == BUTTON_UP && _setupField > 0) _setupField--;
        if (button == BUTTON_DOWN && _setupField < 2) _setupField++;
        if (button == BUTTON_IN) {
            const char* prompts[] = {"Server", "JID (user@domain)", "Password"};
            int f = _setupField;
            Keyboard::Get().Open(prompts[f], [this, f](const String& val) {
                if (f == 0) _server = val;
                else if (f == 1) _jid = val;
                else _pass = val;
            });
        }
        if (button == BUTTON_KEY1) {
            if (_server.length() > 0 && _jid.length() > 0 && _pass.length() > 0) {
                saveSettings();
                _view = XV_STATUS;
                startConn();
            }
        }
        if (button == BUTTON_BACK) SystemCommon::Get().SetNextApp(&AppMenu::Get());
        break;

    case XV_STATUS:
        if (button == BUTTON_BACK && _progress == -1) {
            stopConn();
            _view = XV_SETUP;
        }
        break;

    case XV_CONTACTS:
        if (button == BUTTON_UP && _selContact > 0) _selContact--;
        if (button == BUTTON_DOWN) {
            xSemaphoreTake(_mutex, portMAX_DELAY);
            int n = _contacts.size();
            xSemaphoreGive(_mutex);
            if (_selContact < n - 1) _selContact++;
        }
        if (button == BUTTON_IN) {
            xSemaphoreTake(_mutex, portMAX_DELAY);
            if (_selContact < (int)_contacts.size())
                _chatJid = _contacts[_selContact];
            xSemaphoreGive(_mutex);
            if (_chatJid.length() > 0) {
                bool hasLocal = false;
                for (auto& m : _msgs) { if (m.jid == _chatJid) { hasLocal = true; break; } }
                if (!hasLocal) loadChatFromSD(_chatJid);
                _view = XV_CHAT;
                _scroll.reset();
                _lastChatCount = 0;
            }
        }
        if (button == BUTTON_KEY1) {
            Keyboard::Get().Open("Enter JID", [this](const String& jid) {
                if (jid.length() > 0) {
                    _chatJid = jid;
                    xSemaphoreTake(_mutex, portMAX_DELAY);
                    bool found = false;
                    for (auto& c : _contacts) { if (c == jid) { found = true; break; } }
                    if (!found) { _contacts.push_back(jid); saveContactsToSD(); }
                    xSemaphoreGive(_mutex);
                    bool hasLocal = false;
                    for (auto& m : _msgs) { if (m.jid == jid) { hasLocal = true; break; } }
                    if (!hasLocal) loadChatFromSD(jid);
                    _view = XV_CHAT;
                    _scroll.reset();
                    _lastChatCount = 0;
                }
            });
        }
        if (button == BUTTON_BACK) SystemCommon::Get().SetNextApp(&MainMenu::Get());
        break;

    case XV_CHAT:
        if (button == BUTTON_BACK) { _view = XV_CONTACTS; _scroll.reset(); }
        if (button == BUTTON_UP) _scroll.scrollY = max(0, _scroll.scrollY - 20);
        if (button == BUTTON_DOWN) _scroll.scrollY += 20;
        if (button == BUTTON_IN) {
            Keyboard::Get().Open("Message", [this](const String& msg) {
                if (msg.length() > 0 && _progress == 7) {
                    _outTo = _chatJid;
                    _outBody = msg;
                    _outPending = true;
                }
            });
        }
        break;
    }
}

void XmppNonStaticApp::UpdateTouch(const TouchPoint* touches, int count) {
    if (count <= 0) return;
    const TouchPoint& tp = touches[0];

    switch (_view) {
    case XV_SETUP:
        if (tp.type == TAP) {
            for (int i = 0; i < 3; i++) {
                int y = 56 + i * 46;
                if (tp.y >= y - 2 && tp.y < y + 42) {
                    _setupField = i;
                    const char* prompts[] = {"Server", "JID (user@domain)", "Password"};
                    int f = i;
                    Keyboard::Get().Open(prompts[f], [this, f](const String& val) {
                        if (f == 0) _server = val;
                        else if (f == 1) _jid = val;
                        else _pass = val;
                    });
                    return;
                }
            }
            if (tp.y >= 210 && tp.y < 240 && tp.x >= 60 && tp.x < 180) {
                if (_server.length() > 0 && _jid.length() > 0 && _pass.length() > 0) {
                    saveSettings();
                    _view = XV_STATUS;
                    startConn();
                }
            }
        }
        break;

    case XV_CONTACTS: {
        int listTop = 44;
        xSemaphoreTake(_mutex, portMAX_DELAY);
        int n = _contacts.size();
        xSemaphoreGive(_mutex);
        _scroll.setContent(n * ITEM_H, SCREEN_HEIGHT - listTop);
        if (!_scroll.handleTouch(tp) && tp.type == TAP) {
            int tapped = (tp.y - listTop + _scroll.scrollY) / ITEM_H;
            xSemaphoreTake(_mutex, portMAX_DELAY);
            if (tapped >= 0 && tapped < (int)_contacts.size()) {
                _selContact = tapped;
                _chatJid = _contacts[tapped];
                xSemaphoreGive(_mutex);
                bool hasLocal = false;
                for (auto& m : _msgs) { if (m.jid == _chatJid) { hasLocal = true; break; } }
                if (!hasLocal) loadChatFromSD(_chatJid);
                _view = XV_CHAT;
                _scroll.reset();
                _lastChatCount = 0;
            } else {
                xSemaphoreGive(_mutex);
            }
        }
        break;
    }

    case XV_CHAT: {
        xSemaphoreTake(_mutex, portMAX_DELAY);
        int msgCt = 0;
        for (auto& m : _msgs) if (m.jid == _chatJid) msgCt++;
        xSemaphoreGive(_mutex);
        _scroll.setContent(msgCt * 14, SCREEN_HEIGHT - 60);
        _scroll.handleTouch(tp);
        break;
    }

    default: break;
    }
}

// ========== Icon ==========

const uint16_t* XmppNonStaticApp::getIcon() { return xmpp_icon; }
const uint16_t* XmppNonStaticApp::StaticIcon() { return xmpp_icon; }
