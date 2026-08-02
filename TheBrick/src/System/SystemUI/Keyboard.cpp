#include "System/SystemUI/Keyboard.h"
#include "System/systemDefines.h"

Keyboard& Keyboard::Get() {
    static Keyboard instance;
    return instance;
}

void Keyboard::Open(const char* prompt, std::function<void(const String&)> callback) {
    _prompt = prompt;
    _callback = callback;
    _text = "";
    _mode = LOWER;
    _open = true;
}

void Keyboard::Close() {
    _open = false;
    _callback = nullptr;
}

void Keyboard::submit() {
    String result = _text;
    auto cb = _callback;
    Close();
    if (cb) cb(result);
}

// Row data per mode: row0 (10 chars), row1 (9 chars), row2 (7 chars)
static const char* R0[] = { "qwertyuiop", "QWERTYUIOP", "1234567890" };
static const char* R1[] = { "asdfghjkl",  "ASDFGHJKL",  "@#$%&-+()" };
static const char* R2[] = { "zxcvbnm",    "ZXCVBNM",    "!?=*/\\:" };

char Keyboard::keyAt(int row, int col) const {
    int m = (int)_mode;
    if (row == 0 && col < 10) return R0[m][col];
    if (row == 1 && col < 9)  return R1[m][col];
    if (row == 2 && col < 7)  return R2[m][col];
    return 0;
}

void Keyboard::drawKey(TFT_eSprite& spr, int x, int y, int w, int h,
                       const char* label, uint16_t bg) {
    spr.fillRoundRect(x + 1, y + 1, w - 2, h - 2, 4, bg);
    spr.setTextColor(TFT_WHITE);
    spr.setTextDatum(MC_DATUM);
    spr.setTextSize(1);
    spr.drawString(label, x + w / 2, y + h / 2);
    spr.setTextDatum(TL_DATUM);
}

void Keyboard::Draw(TFT_eSprite& spr) {
    if (!_open) return;

    // Background
    spr.fillRect(0, 0, 240, 320, 0x0841);

    // Prompt
    spr.setTextColor(0x7BEF);
    spr.setTextSize(1);
    spr.drawString(_prompt.c_str(), 10, 10);

    // Input text
    spr.setTextColor(TFT_WHITE);
    spr.setTextSize(2);
    int maxChars = 18;
    String display = _text;
    if ((int)display.length() > maxChars)
        display = display.substring(display.length() - maxChars);
    spr.drawString(display.c_str(), 10, 38);
    int curX = 10 + spr.textWidth(display.c_str());
    spr.fillRect(curX + 2, 36, 2, 22, TFT_WHITE);

    // Separator
    spr.drawFastHLine(0, TEXT_H - 1, 240, 0x3186);

    const uint16_t KEY_BG  = 0x2945;
    const uint16_t SPEC_BG = 0x18C3;
    const uint16_t OK_BG   = 0x0320;

    char lbl[2] = {0, 0};

    // Row 0: 10 keys, 24px wide each
    int y0 = TEXT_H;
    for (int i = 0; i < 10; i++) {
        lbl[0] = keyAt(0, i);
        drawKey(spr, i * 24, y0, 24, ROW_H, lbl, KEY_BG);
    }

    // Row 1: 9 keys, offset 12px
    int y1 = TEXT_H + ROW_H;
    for (int i = 0; i < 9; i++) {
        lbl[0] = keyAt(1, i);
        drawKey(spr, 12 + i * 24, y1, 24, ROW_H, lbl, KEY_BG);
    }

    // Row 2: shift(36) + 7 keys(24) + backspace(36)
    int y2 = TEXT_H + ROW_H * 2;
    drawKey(spr, 0, y2, 36, ROW_H, _mode == UPPER ? "^" : "^",
            _mode == UPPER ? 0x4208 : SPEC_BG);
    for (int i = 0; i < 7; i++) {
        lbl[0] = keyAt(2, i);
        drawKey(spr, 36 + i * 24, y2, 24, ROW_H, lbl, KEY_BG);
    }
    drawKey(spr, 204, y2, 36, ROW_H, "<x", SPEC_BG);

    // Row 3: mode(48) + _(24) + space(96) + .(24) + OK(48)
    int y3 = TEXT_H + ROW_H * 3;
    drawKey(spr, 0,   y3, 48, ROW_H, _mode == SYMBOLS ? "abc" : "123", SPEC_BG);
    drawKey(spr, 48,  y3, 24, ROW_H, "_", KEY_BG);
    drawKey(spr, 72,  y3, 96, ROW_H, "   ", KEY_BG);
    drawKey(spr, 168, y3, 24, ROW_H, ".", KEY_BG);
    drawKey(spr, 192, y3, 48, ROW_H, "OK", OK_BG);
}

bool Keyboard::HandleTouch(const TouchPoint& tp) {
    if (!_open) return false;
    if (tp.type != TAP) return true;

    int tx = tp.x, ty = tp.y;
    if (ty < TEXT_H) return true;

    int row = (ty - TEXT_H) / ROW_H;
    if (row > 3) return true;

    if (row == 0) {
        int col = tx / 24;
        if (col >= 0 && col < 10) {
            char c = keyAt(0, col);
            if (c) _text += c;
            if (_mode == UPPER) _mode = LOWER;
        }
    } else if (row == 1) {
        int col = (tx - 12) / 24;
        if (col >= 0 && col < 9) {
            char c = keyAt(1, col);
            if (c) _text += c;
            if (_mode == UPPER) _mode = LOWER;
        }
    } else if (row == 2) {
        if (tx < 36) {
            _mode = (_mode == UPPER) ? LOWER : UPPER;
        } else if (tx >= 204) {
            if (_text.length() > 0)
                _text.remove(_text.length() - 1);
        } else {
            int col = (tx - 36) / 24;
            if (col >= 0 && col < 7) {
                char c = keyAt(2, col);
                if (c) _text += c;
                if (_mode == UPPER) _mode = LOWER;
            }
        }
    } else if (row == 3) {
        if (tx < 48) {
            _mode = (_mode == SYMBOLS) ? LOWER : SYMBOLS;
        } else if (tx < 72) {
            _text += '_';
        } else if (tx < 168) {
            _text += ' ';
        } else if (tx < 192) {
            _text += '.';
        } else {
            submit();
        }
    }

    return true;
}

bool Keyboard::HandleButton(int button) {
    if (!_open) return false;

    if (button == BUTTON_BACK) {
        Close();
        return true;
    }
    if (button == BUTTON_IN) {
        submit();
        return true;
    }
    return true;
}
