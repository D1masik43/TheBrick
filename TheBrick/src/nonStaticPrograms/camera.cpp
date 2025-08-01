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
    // Nothing
}

void CameraNonStaticApp::Setup() {
  screenBuff = &SystemDrivers::Get().GetScreenBuff();

      // === Camera config ===
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
  config.pixel_format = PIXFORMAT_RGB565;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.grab_mode = CAMERA_GRAB_LATEST;
  } else {
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
    config.fb_location = CAMERA_FB_IN_DRAM;
  }

  // === Init camera ===
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed! 0x%x\n", err);
    screenBuff->fillScreen(TFT_RED);
    screenBuff->drawString("Camera FAIL", 10, 30);
    return;
  }

  sensor_t *s = esp_camera_sensor_get();
  s->set_vflip(s, 1);  // Flip if image is upside down
  s->set_hmirror(s, 1); 

  screenBuff->fillScreen(TFT_GREEN);
  screenBuff->drawString("Camera OK", 10, 30);
}

void CameraNonStaticApp::Draw() {

    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Camera capture failed");
        return;
    }

    if (fb->format == PIXFORMAT_RGB565) {
    // Frame size from camera
    int camWidth = fb->width;
    int camHeight = fb->height;

    // Optional: center the image on the display
    int xOffset = (SCREEN_WIDTH - camWidth) / 2;
    int yOffset = (SCREEN_HEIGHT- camHeight) / 2;

    // Cast buffer to uint16_t* because RGB565 = 2 bytes per pixel
    uint16_t *img = (uint16_t *)fb->buf;

    // Draw directly
    screenBuff->pushImage(xOffset, yOffset, camWidth, camHeight, img);
    }
    esp_camera_fb_return(fb);
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