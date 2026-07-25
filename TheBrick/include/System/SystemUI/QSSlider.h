#pragma once
#include <TFT_eSPI.h>
#include "System/systemStructs.h"

class QSSlider {
public:
    QSSlider(int x, int y, int w, int h, const char* label, int minVal, int maxVal, int value);

    void Draw(TFT_eSprite& sprite, int offsetY = 0);
    bool HandleTouch(const TouchPoint& tp);

    int GetValue() const { return _value; }
    void SetValue(int v);

private:
    int _x, _y, _w, _h;
    const char* _label;
    int _vmin, _vmax, _value;
};
