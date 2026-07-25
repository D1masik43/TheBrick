#pragma once
#include "System/systemStructs.h"
#include "System/systemDefines.h"

class Scrollable {
public:
    int scrollY = 0;

    void setContent(int contentHeight, int viewHeight) {
        _contentH = contentHeight;
        _viewH = viewHeight;
    }

    bool handleTouch(const TouchPoint& tp) {
        if (tp.type == SLIDE_BEGIN) {
            _lastY = tp.y;
            _sliding = true;
            return true;
        }
        if (tp.type == SLIDE && _sliding) {
            scrollY += _lastY - tp.y;
            clamp();
            _lastY = tp.y;
            return true;
        }
        if (tp.type == SLIDE_END) {
            _sliding = false;
            return true;
        }
        return false;
    }

    void reset() { scrollY = 0; }

    void clamp() {
        if (scrollY < 0) scrollY = 0;
        int maxS = _contentH - _viewH;
        if (maxS < 0) maxS = 0;
        if (scrollY > maxS) scrollY = maxS;
    }

private:
    int _contentH = 0;
    int _viewH = SCREEN_HEIGHT;
    int _lastY = 0;
    bool _sliding = false;
};
