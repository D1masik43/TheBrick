#pragma once
#include <TFT_eSPI.h>
#include "System/systemStructs.h"
#include <functional>

class Keyboard {
public:
    static Keyboard& Get();

    void Open(const char* prompt, std::function<void(const String&)> callback);
    void Close();
    bool IsOpen() const { return _open; }

    void Draw(TFT_eSprite& spr);
    bool HandleTouch(const TouchPoint& tp);
    bool HandleButton(int button);

private:
    Keyboard() = default;

    bool _open = false;
    String _text;
    String _prompt;
    std::function<void(const String&)> _callback;

    enum Mode { LOWER, UPPER, SYMBOLS };
    Mode _mode = LOWER;

    static const int TEXT_H = 80;
    static const int ROW_H  = 60;

    void submit();
    void drawKey(TFT_eSprite& spr, int x, int y, int w, int h,
                 const char* label, uint16_t bg);
    char keyAt(int row, int col) const;
};
