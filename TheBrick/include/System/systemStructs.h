#pragma once

enum TouchType {
    NONE,
    TAP,
    SLIDE,
    SLIDE_BEGIN,
    SLIDE_END
};

struct TouchPoint {
    int x;
    int y;
    TouchType type;

    TouchPoint(int x = 0, int y = 0, TouchType type = NONE)
        : x(x), y(y), type(type) {}
};
