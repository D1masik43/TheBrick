#include "nonStaticPrograms/camera.h"
#include "staticPrograms/mainMenu.h"

// Optionally define a static icon
static const uint16_t camera_icon[16 * 16 / 8] = { 0 };  // blank 16x16 monochrome

CameraNonStaticApp::CameraNonStaticApp(const std::string& name)
    : NonStaticApp(name) {}

void CameraNonStaticApp::Loop() {
    Draw();
}

void CameraNonStaticApp::UpdateButtons(int button) {
    switch(button) {
        case BUTTON_UP:

            break;
        case BUTTON_DOWN:

            break;
        case BUTTON_LEFT:

            break;
        case BUTTON_RIGHT:

            break;
        case BUTTON_IN:
                TakePicture();
            break;
        case BUTTON_BACK:
            SystemCommon::Get().SetNextApp(&AppMenu::Get());
            break;
        case BUTTON_HOME:
            SystemCommon::Get().SetNextApp(&MainMenu::Get());
            break;
        case BUTTON_KEY1:

            break;
        case BUTTON_KEY2:

            break;
        default:

        break;
    }
}

void CameraNonStaticApp::UpdateTouch(const TouchPoint* touches, int count) {
    for (int i = 0; i < count; i++) {
        int btnId = takePhotoButton.IsPressed(touches[i]);
        if (btnId == 1) {
            TakePicture();
        }
    }
}

void CameraNonStaticApp::Setup() {
    screenBuff = &SystemDrivers::Get().GetScreenBuff();

    camera_config_t config = {};
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;

    config.pixel_format = PIXFORMAT_RGB565; // always RGB565 for drawing
    config.frame_size = FRAMESIZE_SVGA;     // high-res for JPEG capture
    config.fb_count = psramFound() ? 2 : 1;
    config.fb_location = psramFound() ? CAMERA_FB_IN_PSRAM : CAMERA_FB_IN_DRAM;
    config.grab_mode = CAMERA_GRAB_LATEST;
    config.jpeg_quality = 12;

    if (esp_camera_init(&config) != ESP_OK) {
        screenBuff->fillScreen(TFT_RED);
        screenBuff->drawString("Camera FAIL", 10, 30);
        return;
    }

    sensor_t *s = esp_camera_sensor_get();
    s->set_vflip(s, 1);
    s->set_hmirror(s, 1);

    screenBuff->fillScreen(TFT_GREEN);
    screenBuff->drawString("Camera OK", 10, 30);
}

void CameraNonStaticApp::Draw() {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) return;

    // Allocate TFT buffer on heap
    uint16_t* tftBuf = new uint16_t[240*240]; // ~115 KB
    if(!tftBuf){
        esp_camera_fb_return(fb);
        Serial.println("Failed to allocate TFT buffer");
        return;
    }

    int srcW = fb->width;
    int srcH = fb->height;
    uint16_t *src = (uint16_t*)fb->buf;

    for(int y=0;y<240;y++){
        int sy = y*srcH/240;
        for(int x=0;x<240;x++){
            int sx = x*srcW/240;
            tftBuf[y*240 + x] = src[sy*srcW + sx];
        }
    }

    screenBuff->pushImage(0, 0, 240, 240, tftBuf);
    delete[] tftBuf; // free memory
    esp_camera_fb_return(fb);

    takePhotoButton.Draw(*screenBuff);
}


const uint16_t* CameraNonStaticApp::getIcon() {
    return camera_icon;
}

const uint16_t* CameraNonStaticApp::StaticIcon() {
    return camera_icon;
}

void CameraNonStaticApp::CloseApp() {
    esp_camera_deinit();
}

void CameraNonStaticApp::TakePicture() {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Capture failed");
        return;
    }

    // --- convert to JPEG ---
    uint8_t *jpegBuf = nullptr;
    size_t jpegLen = 0;
    bool ok = fmt2jpg((uint8_t*)fb->buf, fb->width*fb->height*2,
                      fb->width, fb->height,
                      PIXFORMAT_RGB565, 90, &jpegBuf, &jpegLen);
    esp_camera_fb_return(fb);

    if(!ok){
        Serial.println("JPEG encoding failed");
        return;
    }

    char filename[32];
    sprintf(filename, "/photo_%lu.jpg", millis());
    fs::File file = SD_MMC.open(filename, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file");
        free(jpegBuf);
        return;
    }

    file.write(jpegBuf, jpegLen);
    file.close();
    free(jpegBuf);
    Serial.printf("Saved JPEG: %s\n", filename);
    screenBuff->setCursor(20, 250);
    screenBuff->println(filename);
}

/*void CameraNonStaticApp::TakePicture() {
    // Temporarily switch to JPEG mode
    sensor_t *s = esp_camera_sensor_get();
    esp_camera_deinit(); // Stop current capture

    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;

    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG; // JPEG for capture
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;

    if (psramFound()) config.fb_location = CAMERA_FB_IN_PSRAM;
    else config.fb_location = CAMERA_FB_IN_DRAM;

    if (esp_camera_init(&config) != ESP_OK) {
        Serial.println("JPEG camera init failed!");
        return;
    }

    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Failed to capture JPEG");
        return;
    }

    // Build filename
    char filename[32];
    sprintf(filename, "/photo_%lu.jpg", millis());
    fs::File file = SD_MMC.open(filename, FILE_WRITE);
    if (!file) {
        Serial.println("Failed to open file for writing");
        esp_camera_fb_return(fb);
        return;
    }

    file.write(fb->buf, fb->len);
    file.close();
    esp_camera_fb_return(fb);
    Serial.printf("Saved JPEG: %s\n", filename);

    // Reinitialize camera back to RGB565 for display
    config.pixel_format = PIXFORMAT_RGB565;
    esp_camera_deinit();
    esp_camera_init(&config);
}*/

/*void CameraNonStaticApp::TakePicture() {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Capture failed");
        return;
    }

    // --- convert to JPEG ---
    uint8_t *jpegBuf = nullptr;
    size_t jpegLen = 0;
    bool ok = fmt2jpg((uint8_t*)fb->buf, fb->width*fb->height*2,
                      fb->width, fb->height,
                      PIXFORMAT_RGB565, 90, &jpegBuf, &jpegLen);
    esp_camera_fb_return(fb);

    if(!ok){
        Serial.println("JPEG encoding failed");
        return;
    }

    char filename[32];
    sprintf(filename, "/photo_%lu.jpg", millis());
    fs::File file = SD_MMC.open(filename, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file");
        free(jpegBuf);
        return;
    }

    file.write(jpegBuf, jpegLen);
    file.close();
    free(jpegBuf);
    Serial.printf("Saved JPEG: %s\n", filename);
} */