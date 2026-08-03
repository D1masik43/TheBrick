#include "System/SystemUI/Keyboard.h"
#include "System/systemDefines.h"
#include "fonts/cyrillic_draw.h"

Keyboard& Keyboard::Get() {
    static Keyboard instance;
    return instance;
}

void Keyboard::Open(const char* prompt, std::function<void(const String&)> callback) {
    _prompt = prompt;
    _callback = callback;
    _text = "";
    _mode = LOWER;
    _preSymMode = LOWER;
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

// English layouts (indexed by char position)
static const char* EN_R0[] = { "q","w","e","r","t","y","u","i","o","p" };
static const char* EN_R0_U[] = { "Q","W","E","R","T","Y","U","I","O","P" };
static const char* EN_R1[] = { "a","s","d","f","g","h","j","k","l" };
static const char* EN_R1_U[] = { "A","S","D","F","G","H","J","K","L" };
static const char* EN_R2[] = { "z","x","c","v","b","n","m" };
static const char* EN_R2_U[] = { "Z","X","C","V","B","N","M" };

static const char* SYM_R0[] = { "1","2","3","4","5","6","7","8","9","0" };
static const char* SYM_R1[] = { "@","#","$","%","&","-","+","(",")" };
static const char* SYM_R2[] = { "!","?","=","*","/","\\",":" };

// Ukrainian layouts (ЙЦУКЕН)
static const char* UA_R0[] =   { "й","ц","у","к","е","н","г","ш","щ","з","х","ї" };
static const char* UA_R0_U[] = { "Й","Ц","У","К","Е","Н","Г","Ш","Щ","З","Х","Ї" };
static const char* UA_R1[] =   { "ф","і","в","а","п","р","о","л","д","ж","є" };
static const char* UA_R1_U[] = { "Ф","І","В","А","П","Р","О","Л","Д","Ж","Є" };
static const char* UA_R2[] =   { "я","ч","с","м","и","т","ь","б","ю" };
static const char* UA_R2_U[] = { "Я","Ч","С","М","И","Т","Ь","Б","Ю" };

int Keyboard::keysInRow(int row) const {
    if (_mode == SYMBOLS) {
        return row == 0 ? 10 : row == 1 ? 9 : 7;
    }
    if (isUA()) {
        return row == 0 ? 12 : row == 1 ? 11 : 9;
    }
    return row == 0 ? 10 : row == 1 ? 9 : 7;
}

int Keyboard::keyWidth(int row) const {
    if (isUA()) return 20;
    return 24;
}

int Keyboard::rowOffset(int row) const {
    if (row == 0) return 0;
    int total = keysInRow(row) * keyWidth(row);
    return (240 - total) / 2;
}

const char* Keyboard::keyAt(int row, int col) const {
    int n = keysInRow(row);
    if (col < 0 || col >= n) return "";

    switch (_mode) {
    case LOWER:
        if (row == 0) return EN_R0[col];
        if (row == 1) return EN_R1[col];
        return EN_R2[col];
    case UPPER:
        if (row == 0) return EN_R0_U[col];
        if (row == 1) return EN_R1_U[col];
        return EN_R2_U[col];
    case SYMBOLS:
        if (row == 0) return SYM_R0[col];
        if (row == 1) return SYM_R1[col];
        return SYM_R2[col];
    case UA_LOWER:
        if (row == 0) return UA_R0[col];
        if (row == 1) return UA_R1[col];
        return UA_R2[col];
    case UA_UPPER:
        if (row == 0) return UA_R0_U[col];
        if (row == 1) return UA_R1_U[col];
        return UA_R2_U[col];
    }
    return "";
}

void Keyboard::removeLastChar() {
    int len = _text.length();
    if (len == 0) return;
    int i = len - 1;
    while (i > 0 && (_text[i] & 0xC0) == 0x80) i--;
    _text.remove(i);
}

void Keyboard::drawKey(TFT_eSprite& spr, int x, int y, int w, int h,
                       const char* label, uint16_t bg) {
    spr.fillRoundRect(x + 1, y + 1, w - 2, h - 2, 4, bg);
    spr.setTextSize(1);
    if (hasCyrillic(label)) {
        int tw = textWidthVLW(label);
        int fh = vlwRead32(cyrillic_font + 8);
        drawStringVLW(spr, label, x + w / 2 - tw / 2, y + h / 2 - fh / 2, TFT_WHITE);
    } else {
        spr.setTextColor(TFT_WHITE);
        spr.setTextDatum(MC_DATUM);
        spr.drawString(label, x + w / 2, y + h / 2);
        spr.setTextDatum(TL_DATUM);
    }
}

void Keyboard::Draw(TFT_eSprite& spr) {
    if (!_open) return;

    spr.fillRect(0, 0, 240, 320, 0x0841);

    // Prompt
    spr.setTextColor(0x7BEF);
    spr.setTextSize(1);
    spr.drawString(_prompt.c_str(), 10, 10);

    // Input text
    spr.setTextSize(1);
    String display = _text;
    int maxW = 220;
    bool cyr = hasCyrillic(display.c_str());
    while (display.length() > 0) {
        int tw = cyr ? textWidthVLW(display.c_str()) : spr.textWidth(display.c_str());
        if (tw <= maxW) break;
        int i = 1;
        while (i < (int)display.length() && (display[i] & 0xC0) == 0x80) i++;
        display = display.substring(i);
    }
    drawStringAuto(spr, display.c_str(), 10, 38, TFT_WHITE);
    int curX = 10 + (cyr ? textWidthVLW(display.c_str()) : spr.textWidth(display.c_str()));
    spr.fillRect(curX + 2, 36, 2, 16, TFT_WHITE);

    spr.drawFastHLine(0, TEXT_H - 1, 240, 0x3186);

    const uint16_t KEY_BG  = 0x2945;
    const uint16_t SPEC_BG = 0x18C3;
    const uint16_t OK_BG   = 0x0320;

    // Row 0
    int y0 = TEXT_H;
    int kw0 = keyWidth(0);
    for (int i = 0; i < keysInRow(0); i++) {
        drawKey(spr, i * kw0, y0, kw0, ROW_H, keyAt(0, i), KEY_BG);
    }

    // Row 1
    int y1 = TEXT_H + ROW_H;
    int kw1 = keyWidth(1);
    int off1 = rowOffset(1);
    for (int i = 0; i < keysInRow(1); i++) {
        drawKey(spr, off1 + i * kw1, y1, kw1, ROW_H, keyAt(1, i), KEY_BG);
    }

    // Row 2: shift + keys + backspace
    int y2 = TEXT_H + ROW_H * 2;
    int n2 = keysInRow(2);
    int kw2 = keyWidth(2);
    int keysArea = n2 * kw2;
    int sideW = (240 - keysArea) / 2;

    drawKey(spr, 0, y2, sideW, ROW_H,
            (_mode == UPPER || _mode == UA_UPPER) ? "^" : "^",
            (_mode == UPPER || _mode == UA_UPPER) ? 0x4208 : SPEC_BG);
    for (int i = 0; i < n2; i++) {
        drawKey(spr, sideW + i * kw2, y2, kw2, ROW_H, keyAt(2, i), KEY_BG);
    }
    drawKey(spr, sideW + keysArea, y2, 240 - sideW - keysArea, ROW_H, "<x", SPEC_BG);

    // Row 3: mode + lang + space + . + OK
    int y3 = TEXT_H + ROW_H * 3;
    const char* modeLabel = (_mode == SYMBOLS) ? "abc" : "123";
    drawKey(spr, 0,   y3, 48, ROW_H, modeLabel, SPEC_BG);
    drawKey(spr, 48,  y3, 24, ROW_H, isUA() ? "EN" : "UA", SPEC_BG);
    drawKey(spr, 72,  y3, 96, ROW_H, " ",  KEY_BG);
    drawKey(spr, 168, y3, 24, ROW_H, ".",  KEY_BG);
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
        int kw = keyWidth(0);
        int col = tx / kw;
        if (col >= 0 && col < keysInRow(0)) {
            _text += keyAt(0, col);
            if (_mode == UPPER) _mode = LOWER;
            else if (_mode == UA_UPPER) _mode = UA_LOWER;
        }
    } else if (row == 1) {
        int kw = keyWidth(1);
        int off = rowOffset(1);
        int col = (tx - off) / kw;
        if (col >= 0 && col < keysInRow(1)) {
            _text += keyAt(1, col);
            if (_mode == UPPER) _mode = LOWER;
            else if (_mode == UA_UPPER) _mode = UA_LOWER;
        }
    } else if (row == 2) {
        int n2 = keysInRow(2);
        int kw = keyWidth(2);
        int keysArea = n2 * kw;
        int sideW = (240 - keysArea) / 2;

        if (tx < sideW) {
            // Shift
            if (_mode == LOWER) _mode = UPPER;
            else if (_mode == UPPER) _mode = LOWER;
            else if (_mode == UA_LOWER) _mode = UA_UPPER;
            else if (_mode == UA_UPPER) _mode = UA_LOWER;
        } else if (tx >= sideW + keysArea) {
            // Backspace
            removeLastChar();
        } else {
            int col = (tx - sideW) / kw;
            if (col >= 0 && col < n2) {
                _text += keyAt(2, col);
                if (_mode == UPPER) _mode = LOWER;
                else if (_mode == UA_UPPER) _mode = UA_LOWER;
            }
        }
    } else if (row == 3) {
        if (tx < 48) {
            // Mode: 123/abc
            if (_mode == SYMBOLS) {
                _mode = _preSymMode;
            } else {
                _preSymMode = _mode;
                _mode = SYMBOLS;
            }
        } else if (tx < 72) {
            // Language toggle
            if (_mode == LOWER || _mode == UPPER) _mode = UA_LOWER;
            else if (_mode == UA_LOWER || _mode == UA_UPPER) _mode = LOWER;
            else if (_mode == SYMBOLS) { _preSymMode = UA_LOWER; }
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
