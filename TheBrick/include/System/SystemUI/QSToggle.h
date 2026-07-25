#pragma once
#include <TFT_eSPI.h>
#include "System/systemStructs.h"

class QSToggle {
public:
    QSToggle(int x, int y, int size, const char* label, bool enabled = false);

    void Draw(TFT_eSprite& sprite, int offsetY = 0);
    bool HandleTouch(const TouchPoint& tp);

    bool IsEnabled() const { return _enabled; }
    void SetEnabled(bool en) { _enabled = en; }

private:
    int _x, _y, _size;
    const char* _label;
    bool _enabled;
};
