// files.cpp
#include "nonStaticPrograms/files.h"
#include "staticPrograms/mainMenu.h"
#include "staticPrograms/appMenu.h"
#include <esp_heap_caps.h>
#include <cmath>
#include <algorithm>

// Static pointers for TJpgDec callback (no captures allowed)
static uint16_t* s_decBuf = nullptr;
static int       s_decW   = 0;
static int       s_decH   = 0;

static bool jpgCallback(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bmp) {
    for (int row = 0; row < h; row++) {
        int dy = y + row;
        if (dy < 0 || dy >= s_decH) continue;
        int copyX = x, copyW = w;
        if (copyX < 0) { copyW += copyX; copyX = 0; }
        if (copyX + copyW > s_decW) copyW = s_decW - copyX;
        if (copyW <= 0) continue;
        memcpy(&s_decBuf[dy * s_decW + copyX],
               &bmp[row * w + (copyX - x)],
               copyW * sizeof(uint16_t));
    }
    return true;
}

static void getJpgSize(const uint8_t* buf, size_t len, int& w, int& h) {
    w = h = 0;
    for (size_t i = 0; i + 9 < len; i++) {
        if (buf[i] == 0xFF && buf[i+1] >= 0xC0 && buf[i+1] <= 0xC3) {
            h = (buf[i+5] << 8) | buf[i+6];
            w = (buf[i+7] << 8) | buf[i+8];
            return;
        }
    }
}

// ---------------------------------------------------------------------------

FilesNonStaticApp::FilesNonStaticApp(const std::string& name)
    : NonStaticApp(name) {}

void FilesNonStaticApp::Setup() {
    screenBuff = &SystemDrivers::Get().GetScreenBuff();
    if (!SD_MMC.begin("/sdcard", true)) {
        Serial.println("SD_MMC mount failed!");
        return;
    }
    listFiles();
    maxOffsetY = std::max(0, (int)fileList.size() * itemHeight - screenBuff->height());
}

void FilesNonStaticApp::Loop() {
    Draw();
}

void FilesNonStaticApp::CloseApp() {
    closeImage();
}

// ---------------------------------------------------------------------------
// Buttons
// ---------------------------------------------------------------------------
void FilesNonStaticApp::UpdateButtons(int button) {
    if (displayingImage) {
        if (button == BUTTON_BACK) { closeImage(); return; }
        if (button == BUTTON_HOME) {
            closeImage();
            SystemCommon::Get().SetNextApp(&MainMenu::Get());
            return;
        }
        const float panStep = 40.0f;
        if (button == BUTTON_UP)    { panY -= panStep; clampPan(); needsRedraw = true; }
        if (button == BUTTON_DOWN)  { panY += panStep; clampPan(); needsRedraw = true; }
        if (button == BUTTON_LEFT)  { panX -= panStep; clampPan(); needsRedraw = true; }
        if (button == BUTTON_RIGHT) { panX += panStep; clampPan(); needsRedraw = true; }
        return;
    }
    if (button == BUTTON_BACK) SystemCommon::Get().SetNextApp(&AppMenu::Get());
    if (button == BUTTON_HOME) SystemCommon::Get().SetNextApp(&MainMenu::Get());
}

// ---------------------------------------------------------------------------
// Touch
// ---------------------------------------------------------------------------
void FilesNonStaticApp::UpdateTouch(const TouchPoint* touches, int count) {
    if (displayingImage) {
        bool t0 = count > 0 && touches[0].type != NONE;
        bool t1 = count > 1 && touches[1].type != NONE;

        if (t0 && t1) {
            float dx   = touches[0].x - touches[1].x;
            float dy   = touches[0].y - touches[1].y;
            float dist = sqrtf(dx*dx + dy*dy);

            if (!wasPinching) {
                wasPinching    = true;
                lastPinchDist  = dist;
                pinchStartZoom = zoom;
                isPanning      = false;

                float cx = (touches[0].x + touches[1].x) * 0.5f;
                float cy = (touches[0].y + touches[1].y) * 0.5f;
                pinchCenterImgX = panX + cx / zoom;
                pinchCenterImgY = panY + cy / zoom;
            } else {
                float newZoom = pinchStartZoom * (dist / lastPinchDist);
                newZoom = std::max(minZoom, std::min(maxZoom, newZoom));

                float cx = (touches[0].x + touches[1].x) * 0.5f;
                float cy = (touches[0].y + touches[1].y) * 0.5f;

                zoom = newZoom;
                panX = pinchCenterImgX - cx / zoom;
                panY = pinchCenterImgY - cy / zoom;
                clampPan();
                needsRedraw = true;
            }
            isPanning = false;

        } else if (t0) {
            wasPinching = false;

            if (touches[0].type == SLIDE_BEGIN) {
                isPanning      = true;
                lastPanTouchX  = touches[0].x;
                lastPanTouchY  = touches[0].y;
            } else if (touches[0].type == SLIDE && isPanning) {
                int ddx = touches[0].x - lastPanTouchX;
                int ddy = touches[0].y - lastPanTouchY;
                panX -= ddx / zoom;
                panY -= ddy / zoom;
                clampPan();
                lastPanTouchX = touches[0].x;
                lastPanTouchY = touches[0].y;
                needsRedraw = true;
            } else if (touches[0].type == SLIDE_END || touches[0].type == TAP) {
                isPanning   = false;
                wasPinching = false;
                if (touches[0].type == TAP && zoom <= minZoom + 0.05f) {
                    closeImage();
                    return;
                }
            }
        } else {
            wasPinching = false;
        }
        return;
    }

    // File list
    if (count > 0) {
        if (touches[0].type == SLIDE_BEGIN) {
            lastTouchY = touches[0].y;
            isSliding  = true;
        } else if (touches[0].type == SLIDE && isSliding) {
            int delta  = touches[0].y - lastTouchY;
            totalOffsetY = std::max(-maxOffsetY, std::min(0, totalOffsetY + delta));
            lastTouchY = touches[0].y;
        } else if (touches[0].type == SLIDE_END) {
            isSliding = false;
        } else if (touches[0].type == TAP) {
            int idx = (touches[0].y - totalOffsetY) / itemHeight;
            if (idx >= 0 && idx < (int)fileList.size()) {
                selectedIndex = idx;
                openImage(fileList[idx]);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Draw
// ---------------------------------------------------------------------------
void FilesNonStaticApp::Draw() {
    if (displayingImage) {
        if (needsRedraw) {
            drawImage();
            needsRedraw = false;
        }
        // if no redraw needed, the sprite already has the last frame — 
        // main loop's pushSprite will just re-push the same pixels (free)
        return;
    }
    screenBuff->fillSprite(TFT_BLACK);
    drawFileList();
    // note: main loop calls pushSprite, we don't need to here
}

// ---------------------------------------------------------------------------
// File list
// ---------------------------------------------------------------------------
void FilesNonStaticApp::listFiles() {
    fileList.clear();
    File root = SD_MMC.open("/");
    while (true) {
        File entry = root.openNextFile();
        if (!entry) break;
        String name = entry.name();
        if (name.endsWith(".jpg") || name.endsWith(".jpeg") || name.endsWith(".JPG"))
            fileList.push_back(name);
        entry.close();
    }
    root.close();
}

void FilesNonStaticApp::drawFileList() {
    int y = totalOffsetY;
    for (size_t i = 0; i < fileList.size(); i++) {
        screenBuff->setTextColor(((int)i == selectedIndex) ? TFT_YELLOW : TFT_WHITE, TFT_BLACK);
        if (y + itemHeight >= 0 && y < screenBuff->height())
            screenBuff->drawString(fileList[i], 5, y);
        y += itemHeight;
    }
}

// ---------------------------------------------------------------------------
// Open / close
// ---------------------------------------------------------------------------
void FilesNonStaticApp::openImage(const String& filename) {
    closeImage();

    String fullPath = "/" + filename;
    File file = SD_MMC.open(fullPath, FILE_READ);
    if (!file) { Serial.println("Failed to open: " + fullPath); return; }

    jpgLen = file.size();
    jpgBuf = (uint8_t*)heap_caps_malloc(jpgLen, MALLOC_CAP_SPIRAM);
    if (!jpgBuf) { file.close(); return; }
    if (file.read(jpgBuf, jpgLen) != jpgLen) {
        free(jpgBuf); jpgBuf = nullptr; file.close(); return;
    }
    file.close();

    getJpgSize(jpgBuf, jpgLen, imgW, imgH);
    Serial.printf("Image: %dx%d, %u bytes\n", imgW, imgH, jpgLen);

    // zoom=1.0 → 1 image pixel per screen pixel
    // minZoom → whole image fits screen
    float fitX = (float)screenBuff->width()  / (float)imgW;
    float fitY = (float)screenBuff->height() / (float)imgH;
    minZoom = std::min(fitX, fitY);
    maxZoom = 4.0f;
    zoom    = minZoom;
    panX    = 0;
    panY    = 0;

    wasPinching     = false;
    isPanning       = false;
    displayingImage = true;
    needsRedraw     = true;
    curDivisor      = -1;

    int d = chooseDivisor();
    decodeToBuffer(d);
}

void FilesNonStaticApp::clampPan() {
    // At zoom z: visible image region = scrW/zoom × scrH/zoom image pixels
    float visW = (float)screenBuff->width()  / zoom;
    float visH = (float)screenBuff->height() / zoom;

    if (visW >= (float)imgW) panX = 0;
    else panX = std::max(0.0f, std::min(panX, (float)imgW - visW));

    if (visH >= (float)imgH) panY = 0;
    else panY = std::max(0.0f, std::min(panY, (float)imgH - visH));
}

void FilesNonStaticApp::closeImage() {
    if (jpgBuf) { free(jpgBuf); jpgBuf = nullptr; }
    if (decBuf) { free(decBuf); decBuf = nullptr; }
    jpgLen = decBufW = decBufH = 0;
    imgW = imgH = 0;
    curDivisor = -1;
    displayingImage = false;
    needsRedraw     = false;
}

// ---------------------------------------------------------------------------
// Choose TJpgDec divisor based on zoom
// ---------------------------------------------------------------------------
int FilesNonStaticApp::chooseDivisor() const {
    
    int scrW = screenBuff->width();
    int scrH = screenBuff->height();
    
    // How many image pixels are visible across the screen at current zoom?
    float visibleImgW = (float)scrW / zoom;   // image pixels spanning the screen
    float visibleImgH = (float)scrH / zoom;
    

    float maxDivisorF = (float)imgW / (float)scrW;

    int divisor = 1;
    for (int d : {8, 4, 2, 1}) {
        if (imgW / d >= scrW && imgH / d >= scrH) {
            divisor = d;
            break;
        }
    }
    return divisor;
}

// ---------------------------------------------------------------------------
// Decode JPEG → decBuf at given divisor (only called when zoom tier changes)
// ---------------------------------------------------------------------------
void FilesNonStaticApp::decodeToBuffer(int divisor) {
    int newW = std::max(1, imgW / divisor);
    int newH = std::max(1, imgH / divisor);
    size_t needed = (size_t)newW * newH * sizeof(uint16_t);

    // Reallocate only if size changed
    if (newW != decBufW || newH != decBufH || !decBuf) {
        if (decBuf) free(decBuf);
        decBuf = (uint16_t*)heap_caps_malloc(needed, MALLOC_CAP_SPIRAM);
        if (!decBuf) {
            Serial.printf("decBuf alloc failed %u bytes\n", needed);
            decBufW = decBufH = 0;
            return;
        }
        decBufW = newW;
        decBufH = newH;
    }
    memset(decBuf, 0, needed);

    s_decBuf = decBuf;
    s_decW   = newW;
    s_decH   = newH;

    uint8_t scale = (divisor == 1) ? 0 : (divisor == 2) ? 1 : (divisor == 4) ? 2 : 3;
    TJpgDec.setJpgScale(scale);
    TJpgDec.setSwapBytes(true);
    TJpgDec.setCallback(jpgCallback);
    TJpgDec.drawJpg(0, 0, jpgBuf, jpgLen);

    curDivisor = divisor;
    Serial.printf("Decoded at 1/%d → %dx%d\n", divisor, newW, newH);
}

// ---------------------------------------------------------------------------
// Blit decBuf → screenBuff with current pan/zoom (fast, no JPEG decode)
// ---------------------------------------------------------------------------
void FilesNonStaticApp::blitToScreen() {
    if (!decBuf || !decBufW || !decBufH) return;

    int scrW = screenBuff->width();
    int scrH = screenBuff->height();

    // zoom is absolute: zoom=1.0 means 1 image pixel = 1 screen pixel
    // At zoom=minZoom the whole image fits.
    // Screen pixel (sx,sy) maps to image pixel:
    //   imgX = panX + sx / zoom
    //   imgY = panY + sy / zoom
    // In decoded buffer (divisor d):
    //   decX = imgX / d = panX/d + sx / (zoom * d)
    float decPanX = panX / (float)curDivisor;
    float decPanY = panY / (float)curDivisor;
    float stepX   = 1.0f / (zoom * (float)curDivisor);
    float stepY   = 1.0f / (zoom * (float)curDivisor);

    uint16_t rowBuf[320] = {};
    screenBuff->fillSprite(TFT_BLACK);

    for (int sy = 0; sy < scrH; sy++) {
        int dy = (int)(decPanY + sy * stepY);
        if (dy < 0 || dy >= decBufH) continue;
        const uint16_t* srcRow = &decBuf[dy * decBufW];
        for (int sx = 0; sx < scrW; sx++) {
            int dx = (int)(decPanX + sx * stepX);
            rowBuf[sx] = (dx >= 0 && dx < decBufW) ? srcRow[dx] : 0;
        }
        screenBuff->pushImage(0, sy, scrW, 1, rowBuf);
    }

    char buf[32];
    snprintf(buf, sizeof(buf), "%.2fx", zoom);
    screenBuff->setTextColor(TFT_WHITE, TFT_BLACK);
    screenBuff->drawString(buf, 4, 4);
    screenBuff->pushSprite(0, 0);
}

void FilesNonStaticApp::drawImage() {
    if (!jpgBuf || !decBuf) return;
    blitToScreen();
}