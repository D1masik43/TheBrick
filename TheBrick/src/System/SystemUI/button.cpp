#include "System/SystemUI/button.h"

UIButton::UIButton(int id, int x, int y, int w, int h, const char* label)
    : _id(id), _x(x), _y(y), _w(w), _h(h), _label(label) {}

void UIButton::SetColors(uint16_t textColor, uint16_t fillColor, uint16_t borderColor) {
    _textColor = textColor;
    _fillColor = fillColor;
    _borderColor = borderColor;
}

void UIButton::Draw(TFT_eSprite& sprite) {
    sprite.fillRoundRect(_x, _y, _w, _h, 6, _fillColor);
    sprite.drawRoundRect(_x, _y, _w, _h, 6, _borderColor);

    sprite.setTextDatum(MC_DATUM);
    sprite.setTextColor(_textColor);
    sprite.drawString(_label, _x + _w / 2, _y + _h / 2);
}

int UIButton::IsPressed(const TouchPoint& tp) {
    if (tp.type == TAP || tp.type == SLIDE_BEGIN) {
        if (tp.x >= _x && tp.x <= _x + _w &&
            tp.y >= _y && tp.y <= _y + _h) {
            return _id;
        }
    }
    return -1;
}
