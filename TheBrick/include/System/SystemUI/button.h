#pragma once
#include <TFT_eSPI.h>
#include <Arduino.h>
#include "System/systemStructs.h"

class UIButton {
public:
    UIButton(int id, int x, int y, int w, int h, const char* label);

    void SetColors(uint16_t textColor, uint16_t fillColor, uint16_t borderColor);

    // Draw button into a TFT_eSPI Sprite framebuffer
    void Draw(TFT_eSprite& sprite);

    // Returns button ID if pressed, else -1
    int IsPressed(const TouchPoint& tp);

private:
    int _id;
    int _x, _y, _w, _h;
    const char* _label;

    uint16_t _textColor = TFT_WHITE;
    uint16_t _fillColor = TFT_BLUE;
    uint16_t _borderColor = TFT_WHITE;
};
