#include "System/SystemUI/QuickSettings.h"
#include "System/systemDrivers.h"
#include "System/systemImages.h"

QuickSettings& QuickSettings::Get() {
    static QuickSettings instance;
    return instance;
}

//                               x    y    w    h    label         min max  val
QuickSettings::QuickSettings()
    : _volumeSlider    (10,  30,  220, 30, "Volume",     0, 100, 50),
      _brightnessSlider(10,  65,  220, 30, "Brightness", 0, 100, 80),
      _wifiToggle  (10,  110, 48, "WiFi"),
      _btToggle    (68,  110, 48, "BT"),
      _dndToggle   (126, 110, 48, "DND"),
      _flashToggle (184, 110, 48, "LED")
{}

void QuickSettings::CheckSwipeOpen(const TouchPoint& tp) {
    if (tp.type == SLIDE_BEGIN && tp.y < 20) {
        _swipeTracking = true;
        _swipeStartY = tp.y;
    } else if (tp.type == SLIDE && _swipeTracking) {
        int delta = tp.y - _swipeStartY;
        if (!_open && delta > 40) {
            _open = true;
            _animating = true;
            _targetY = PANEL_OPEN_Y;
            _swipeTracking = false;
        }
    } else if (tp.type == SLIDE_END) {
        _swipeTracking = false;
    }
}

void QuickSettings::UpdateTouch(const TouchPoint* touches, int count) {
    if (count <= 0) return;
    const TouchPoint& tp = touches[0];

    if (tp.type == SLIDE_BEGIN && tp.y > _panelY + PANEL_H - 10) {
        _swipeTracking = true;
        _swipeStartY = tp.y;
    } else if (tp.type == SLIDE && _swipeTracking) {
        int delta = _swipeStartY - tp.y;
        if (_open && delta > 40) {
            _open = false;
            _animating = true;
            _targetY = PANEL_CLOSED_Y;
            _swipeTracking = false;
            return;
        }
    } else if (tp.type == SLIDE_END) {
        _swipeTracking = false;
    }

    if (tp.type == TAP && tp.y > _panelY + PANEL_H) {
        _open = false;
        _animating = true;
        _targetY = PANEL_CLOSED_Y;
        return;
    }

    TouchPoint adjusted = tp;
    adjusted.y = tp.y - _panelY;

    if (_volumeSlider.HandleTouch(adjusted)) return;
    if (_brightnessSlider.HandleTouch(adjusted)) return;

    if (_dndToggle.HandleTouch(adjusted)) {
        _dndEnabled = _dndToggle.IsEnabled();
        if (xSemaphoreTake(sim800Mutex, pdMS_TO_TICKS(200)) == pdTRUE) {
            HardwareSerial& sim800 = SystemDrivers::Get().GetSim800();
            sim800.println(_dndEnabled ? "AT+CRSL=0" : "AT+CRSL=100");
            xSemaphoreGive(sim800Mutex);
        }
        return;
    }

    if (_flashToggle.HandleTouch(adjusted)) {
        _ledEnabled = _flashToggle.IsEnabled();
        if (mcpAvailable && xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(200)) == pdTRUE) {
            SystemDrivers::Get().GetMCP().digitalWrite(LED_PIN, _ledEnabled ? HIGH : LOW);
            xSemaphoreGive(i2cMutex);
        }
        return;
    }

    _wifiToggle.HandleTouch(adjusted);
    _btToggle.HandleTouch(adjusted);
}

void QuickSettings::Draw(TFT_eSprite& sprite) {
    if (_animating) {
        int step = (_targetY - _panelY) / 3;
        if (step == 0) step = (_targetY > _panelY) ? 1 : -1;
        _panelY += step;
        if (abs(_panelY - _targetY) <= 1) {
            _panelY = _targetY;
            _animating = false;
        }
    }

    if (_panelY <= PANEL_CLOSED_Y && !_animating) return;

    int drawY = max(0, _panelY);
    int drawH = min(PANEL_H + _panelY, PANEL_H);
    if (drawH <= 0) return;

    int srcY = (_panelY < 0) ? -_panelY : 0;
    const uint16_t* blurSrc = (const uint16_t*)wallpaperBlurred + srcY * 240;
    sprite.pushImage(0, drawY, 240, drawH, blurSrc);

    // dim the blurred bg
    for (int y = drawY; y < drawY + drawH && y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < 240; x++) {
            uint16_t c = sprite.readPixel(x, y);
            uint8_t r = (c >> 11) & 0x1F;
            uint8_t g = (c >> 5) & 0x3F;
            uint8_t b = c & 0x1F;
            sprite.drawPixel(x, y, ((r / 2) << 11) | ((g / 2) << 5) | (b / 2));
        }
    }

    // handle bar at bottom
    int handleY = _panelY + PANEL_H - 6;
    if (handleY >= 0 && handleY < SCREEN_HEIGHT)
        sprite.fillRoundRect(100, handleY, 40, 4, 2, TFT_LIGHTGREY);

    // draw widgets offset by _panelY
    int oY = _panelY;

    // sliders
    _volumeSlider.Draw(sprite, oY);
    _brightnessSlider.Draw(sprite, oY);

    // toggles
    _wifiToggle.Draw(sprite, oY);
    _btToggle.Draw(sprite, oY);
    _dndToggle.Draw(sprite, oY);
    _flashToggle.Draw(sprite, oY);

    sprite.setTextDatum(TL_DATUM);
}
