#include "System/SystemUI/QSToggle.h"

QSToggle::QSToggle(int x, int y, int size, const char* label, bool enabled)
    : _x(x), _y(y), _size(size), _label(label), _enabled(enabled) {}

bool QSToggle::HandleTouch(const TouchPoint& tp) {
    if (tp.type != TAP) return false;
    if (tp.x >= _x && tp.x <= _x + _size &&
        tp.y >= _y && tp.y <= _y + _size) {
        _enabled = !_enabled;
        return true;
    }
    return false;
}

void QSToggle::Draw(TFT_eSprite& sprite, int offsetY) {
    int dy = _y + offsetY;
    uint16_t bg = _enabled ? TFT_CYAN : TFT_DARKGREY;
    uint16_t fg = _enabled ? TFT_BLACK : TFT_WHITE;

    sprite.fillRoundRect(_x, dy, _size, _size, 8, bg);

    sprite.setTextDatum(MC_DATUM);
    sprite.setTextSize(1);
    sprite.setTextColor(fg);
    sprite.drawString(_label, _x + _size / 2, dy + _size / 2);
}
