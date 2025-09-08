#include "nonStaticPrograms/files.h"
#include "staticPrograms/mainMenu.h"
#include "staticPrograms/appMenu.h"

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
    auto& spr = SystemDrivers::Get().GetScreenBuff();
    spr.pushImage(x, y, w, h, bitmap);
    return true;
}


FilesNonStaticApp::FilesNonStaticApp(const std::string& name)
    : NonStaticApp(name) {}

void FilesNonStaticApp::Setup() {
    screenBuff = &SystemDrivers::Get().GetScreenBuff();

    if(!SD_MMC.begin("/sdcard", true)){
        Serial.println("SD_MMC mount failed!");
        return;
    }

    Serial.println("SD_MMC mounted successfully");

    listFiles();

    TJpgDec.setJpgScale(1);
    TJpgDec.setCallback(tft_output);
}

void FilesNonStaticApp::Loop() {
    Draw();
}

void FilesNonStaticApp::UpdateButtons(int button) {
    if (displayingImage && button == BUTTON_BACK) {
        // Close image and return to file list
        displayingImage = false;
        Draw(); // redraw file list
        return;
    }

    if (button == BUTTON_BACK) {
        SystemCommon::Get().SetNextApp(&AppMenu::Get());
    } else if (button == BUTTON_HOME) {
        SystemCommon::Get().SetNextApp(&MainMenu::Get());
    }
}

void FilesNonStaticApp::UpdateTouch(const TouchPoint* touches, int count) {
    if (displayingImage && count > 0 && touches[0].type == TAP) {
        // Any tap while viewing image closes it
        displayingImage = false;
        Draw(); // redraw file list
        return;
    }

    if (count > 0 && touches[0].type == TAP) {
        int itemHeight = 20;
        int idx = touches[0].y / itemHeight;
        if (idx >= 0 && idx < (int)fileList.size()) {
            selectedIndex = idx;
            openImage(fileList[idx]);
            displayingImage = true;
        }
    }
}


void FilesNonStaticApp::Draw() {
    if (displayingImage) {

        return;
    }

    screenBuff->fillSprite(TFT_BLACK);
    drawFileList();
    screenBuff->pushSprite(0, 0);
}

void FilesNonStaticApp::CloseApp() {}

void FilesNonStaticApp::listFiles() {
    fileList.clear();
    File root = SD_MMC.open("/");
    while (true) {
        File entry = root.openNextFile();
        if (!entry) break;
        String name = entry.name();
        if (name.endsWith(".jpg") || name.endsWith(".jpeg")) {
            fileList.push_back(name);
        }
        entry.close();
    }
    root.close();
}

void FilesNonStaticApp::drawFileList() {
    int y = 0;
    for (size_t i = 0; i < fileList.size(); i++) {
        if ((int)i == selectedIndex)
            screenBuff->setTextColor(TFT_YELLOW, TFT_BLACK);
        else
            screenBuff->setTextColor(TFT_WHITE, TFT_BLACK);

        screenBuff->drawString(fileList[i], 5, y);
        y += 20;
    }
}

#include <esp_heap_caps.h>
void getJpgSize(const uint8_t* buf, size_t len, int &w, int &h) {
    w = h = 0;
    size_t i = 0;
    while (i < len - 9) {
        if (buf[i] == 0xFF && (buf[i+1] >= 0xC0 && buf[i+1] <= 0xC3)) {
            // SOF0/1/2 marker found
            h = (buf[i+5] << 8) | buf[i+6];
            w = (buf[i+7] << 8) | buf[i+8];
            return;
        }
        i++;
    }
}

void FilesNonStaticApp::openImage(const String& filename) {
    screenBuff->fillSprite(TFT_BLACK);

    String fullPath = "/" + filename;
    Serial.println("Opening file: " + fullPath);

    File file = SD_MMC.open(fullPath, FILE_READ);
    if (!file) {
        Serial.println("Failed to open file");
        return;
    }

    size_t fileSize = file.size();
    if (fileSize == 0) {
        Serial.println("File is empty");
        file.close();
        return;
    }

    // Allocate buffer in PSRAM
    uint8_t* jpgBuf = (uint8_t*)heap_caps_malloc(fileSize, MALLOC_CAP_SPIRAM);
    if (!jpgBuf) {
        Serial.println("Failed to allocate PSRAM for JPEG");
        file.close();
        return;
    }

    size_t readBytes = file.read(jpgBuf, fileSize);
    file.close();

    if (readBytes != fileSize) {
        Serial.println("Failed to read entire file");
        free(jpgBuf);
        return;
    }

    // Get JPEG dimensions
    int imgW, imgH;
    getJpgSize(jpgBuf, fileSize, imgW, imgH);
    Serial.printf("JPEG size: %dx%d\n", imgW, imgH);

    uint8_t scale = 0; // 0 = 1/1, 1 = 1/2, 2 = 1/4, 3 = 1/8
    if (imgW > 240 || imgH > 240) {
        if (imgW > 960 || imgH > 960) scale = 3;
        else if (imgW > 480|| imgH > 480) scale = 2;
        else if (imgW > 240 || imgH > 240) scale = 1;
        else scale = 0;
    }
    int scaledW = imgW;
    int scaledH = imgH;
    switch (scale) {
    case 1: scaledW /= 2; scaledH /= 2; break;
    case 2: scaledW /= 4; scaledH /= 4; break;
    case 3: scaledW /= 8; scaledH /= 8; break;
    // case 0: do nothing, full size
    }

    Serial.printf("Scaled JPEG size: %dx%d\n", scaledW, scaledH);
    Serial.println(scale);
    TJpgDec.setJpgScale(scale);
    TJpgDec.setCallback(tft_output);
    TJpgDec.setSwapBytes(true);

    if (TJpgDec.drawJpg(0, 0, jpgBuf, fileSize) != 0) {
        Serial.println("JPEG draw failed");
    }

    free(jpgBuf);
    screenBuff->pushSprite(0, 0);
}
