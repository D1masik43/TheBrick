#pragma once
#include <TFT_eSPI.h>
#include "System/systemStructs.h"
#include "System/SystemUI/QSSlider.h"
#include "System/SystemUI/QSToggle.h"

class QuickSettings {
public:
    static QuickSettings& Get();

    void Draw(TFT_eSprite& sprite);
    void UpdateTouch(const TouchPoint* touches, int count);

    bool IsOpen() const { return _open; }
    bool IsAnimating() const { return _animating; }

    void CheckSwipeOpen(const TouchPoint& tp);

private:
    QuickSettings();
    QuickSettings(const QuickSettings&) = delete;
    QuickSettings& operator=(const QuickSettings&) = delete;

    bool _open = false;
    bool _animating = false;
    int _panelY = -220;
    int _targetY = -220;

    int _swipeStartY = 0;
    bool _swipeTracking = false;

    static const int PANEL_H = 220;
    static const int PANEL_CLOSED_Y = -220;
    static const int PANEL_OPEN_Y = 0;

    bool _dndEnabled = false;
    bool _ledEnabled = false;

    QSSlider _volumeSlider;
    QSSlider _brightnessSlider;

    QSToggle _wifiToggle;
    QSToggle _btToggle;
    QSToggle _dndToggle;
    QSToggle _flashToggle;
};
