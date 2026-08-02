// files.h
#pragma once
#include "appTemplates/nonStaticApp.h"
#include "System/systemGlobals.h"
#include "System/systemDrivers.h"
#include <TJpg_Decoder.h>
#include <SD_MMC.h>
#include <vector>

class FilesNonStaticApp : public NonStaticApp {
public:
    FilesNonStaticApp(const std::string& name);

    void Loop() override;
    void UpdateButtons(int button) override;
    void UpdateTouch(const TouchPoint* touches, int count) override;
    void Setup() override;
    void Draw() override;
    void CloseApp() override;

    const uint16_t* getIcon() override { return nullptr; }
    static const uint16_t* StaticIcon() { return nullptr; }

private:
    TFT_eSprite* screenBuff = nullptr;

    // --- File list state ---
    std::vector<String> fileList;
    int selectedIndex  = -1;
    int totalOffsetY   = 0;
    int maxOffsetY     = 0;
    int lastTouchY     = 0;
    bool isSliding     = false;
    static const int itemHeight = 30;

    // --- Image viewer state ---
    bool displayingImage = false;
    bool needsRedraw     = false;

    uint8_t*  jpgBuf = nullptr;
    size_t    jpgLen = 0;
    int imgW = 0, imgH = 0;

    uint16_t* decBuf  = nullptr;
    int       decBufW = 0, decBufH = 0;
    int       curDivisor = -1;
    String    decodeError;

    float zoom    = 1.0f;
    float minZoom = 0.1f;
    float maxZoom = 8.0f;
    float panX = 0, panY = 0;

    // Pinch
    bool  wasPinching      = false;
    float lastPinchDist    = 0;
    float pinchStartZoom   = 1.0f;
    float pinchCenterImgX  = 0, pinchCenterImgY = 0;

    // Pan
    int  lastPanTouchX = 0, lastPanTouchY = 0;
    bool isPanning     = false;

    void listFiles();
    void drawFileList();
    void openImage(const String& filename);
    void closeImage();
    void decodeToBuffer(int divisor);
    void blitToScreen();
    void drawImage();
    void clampPan();
    int  chooseDivisor() const;
};
