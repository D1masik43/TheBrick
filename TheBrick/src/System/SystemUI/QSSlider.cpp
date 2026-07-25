#include "System/SystemUI/QSSlider.h"

QSSlider::QSSlider(int x, int y, int w, int h, const char* label, int minVal, int maxVal, int value)
    : _x(x), _y(y), _w(w), _h(h), _label(label), _vmin(minVal), _vmax(maxVal), _value(value) {}

void QSSlider::SetValue(int v) {
    if (v < _vmin) v = _vmin;
    if (v > _vmax) v = _vmax;
    _value = v;
}

bool QSSlider::HandleTouch(const TouchPoint& tp) {
    if (tp.type != TAP && tp.type != SLIDE && tp.type != SLIDE_BEGIN)
        return false;
    if (tp.x < _x || tp.x > _x + _w || tp.y < _y || tp.y > _y + _h)
        return false;

    int trackX = _x + 4;
    int trackW = _w - 8;
    int rel = tp.x - trackX;
    if (rel < 0) rel = 0;
    if (rel > trackW) rel = trackW;
    _value = _vmin + (int)((long)rel * (_vmax - _vmin) / trackW);
    return true;
}

void QSSlider::Draw(TFT_eSprite& sprite, int offsetY) {
    int dy = _y + offsetY;
    int trackY = dy + _h / 2;
    int trackH = 6;
    int trackX = _x + 4;
    int trackW = _w - 8;

    sprite.setTextDatum(TL_DATUM);
    sprite.setTextSize(1);
    sprite.setTextColor(TFT_WHITE);
    sprite.drawString(_label, _x + 4, dy + 2);

    char valStr[8];
    snprintf(valStr, sizeof(valStr), "%d", _value);
    sprite.setTextDatum(TR_DATUM);
    sprite.drawString(valStr, _x + _w - 4, dy + 2);

    sprite.fillRoundRect(trackX, trackY - trackH / 2, trackW, trackH, 3, TFT_DARKGREY);

    int fillW = (int)((long)(_value - _vmin) * trackW / (_vmax - _vmin));
    if (fillW > 0)
        sprite.fillRoundRect(trackX, trackY - trackH / 2, fillW, trackH, 3, TFT_CYAN);

    int knobX = trackX + fillW;
    sprite.fillCircle(knobX, trackY, 6, TFT_WHITE);
}
