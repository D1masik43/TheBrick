#include "nonStaticPrograms/sysinfo.h"
#include "staticPrograms/mainMenu.h"
#include "staticPrograms/appMenu.h"

static const uint16_t sysinfo_icon[256] = {0};

static const int TOP_Y = 24;
static const int LINE_H = 12;
static const int TOTAL_LINES = 25;

SysInfoNonStaticApp::SysInfoNonStaticApp(const std::string& name)
    : NonStaticApp(name) {}

void SysInfoNonStaticApp::Setup() {
    screenBuff = &SystemDrivers::Get().GetScreenBuff();
    scroll.setContent(TOTAL_LINES * LINE_H, SCREEN_HEIGHT - TOP_Y);
}

void SysInfoNonStaticApp::Loop() {
    if (millis() - lastUpdate > 500) {
        lastUpdate = millis();
        if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            auto &ina = SystemDrivers::Get().GetINA219();
            inaVoltage = ina.getBusVoltage_V();
            inaCurrent = ina.getCurrent_mA();
            inaPower = ina.getPower_mW();
            inaShunt = ina.getShuntVoltage_mV();

            i2cCount = 0;
            for (uint8_t addr = 1; addr < 127 && i2cCount < 16; addr++) {
                Wire.beginTransmission(addr);
                if (Wire.endTransmission() == 0)
                    i2cAddrs[i2cCount++] = addr;
            }

            xSemaphoreGive(i2cMutex);
        }
    }
    Draw();
}

void SysInfoNonStaticApp::Draw() {
    screenBuff->fillScreen(TFT_BLACK);
    screenBuff->setTextDatum(TL_DATUM);
    screenBuff->setTextColor(TFT_WHITE);
    screenBuff->setTextSize(1);

    int y = TOP_Y - scroll.scrollY;
    char buf[64];

    screenBuff->drawString("=== INA219 ===", 4, y); y += LINE_H;
    snprintf(buf, sizeof(buf), "Bus:    %.2f V", inaVoltage);
    screenBuff->drawString(buf, 4, y); y += LINE_H;
    snprintf(buf, sizeof(buf), "Shunt:  %.2f mV", inaShunt);
    screenBuff->drawString(buf, 4, y); y += LINE_H;
    snprintf(buf, sizeof(buf), "Current:%.1f mA", inaCurrent);
    screenBuff->drawString(buf, 4, y); y += LINE_H;
    snprintf(buf, sizeof(buf), "Power:  %.1f mW", inaPower);
    screenBuff->drawString(buf, 4, y); y += LINE_H;

    y += LINE_H;
    screenBuff->drawString("=== Memory ===", 4, y); y += LINE_H;
    snprintf(buf, sizeof(buf), "Heap free: %u KB", esp_get_free_heap_size() / 1024);
    screenBuff->drawString(buf, 4, y); y += LINE_H;
    snprintf(buf, sizeof(buf), "Heap min:  %u KB", esp_get_minimum_free_heap_size() / 1024);
    screenBuff->drawString(buf, 4, y); y += LINE_H;
    snprintf(buf, sizeof(buf), "PSRAM free:%u KB", heap_caps_get_free_size(MALLOC_CAP_SPIRAM) / 1024);
    screenBuff->drawString(buf, 4, y); y += LINE_H;

    y += LINE_H;
    screenBuff->drawString("=== CPU ===", 4, y); y += LINE_H;
    snprintf(buf, sizeof(buf), "Freq: %u MHz", getCpuFrequencyMhz());
    screenBuff->drawString(buf, 4, y); y += LINE_H;
    snprintf(buf, sizeof(buf), "Chip: %s rev %d", ESP.getChipModel(), ESP.getChipRevision());
    screenBuff->drawString(buf, 4, y); y += LINE_H;
    snprintf(buf, sizeof(buf), "Flash: %u MB", ESP.getFlashChipSize() / 1024 / 1024);
    screenBuff->drawString(buf, 4, y); y += LINE_H;

    y += LINE_H;
    screenBuff->drawString("=== I2C ===", 4, y); y += LINE_H;
    if (i2cCount == 0) {
        screenBuff->drawString("No devices", 4, y); y += LINE_H;
    } else {
        String addrs = "";
        for (int i = 0; i < i2cCount; i++) {
            char hex[6];
            snprintf(hex, sizeof(hex), "0x%02X ", i2cAddrs[i]);
            addrs += hex;
        }
        screenBuff->drawString(addrs, 4, y); y += LINE_H;
    }

    y += LINE_H;
    screenBuff->drawString("=== SD ===", 4, y); y += LINE_H;
    if (sdAvailable) {
        snprintf(buf, sizeof(buf), "Size: %llu MB", SD_MMC.cardSize() / 1024 / 1024);
        screenBuff->drawString(buf, 4, y); y += LINE_H;
        snprintf(buf, sizeof(buf), "Used: %llu MB", SD_MMC.usedBytes() / 1024 / 1024);
        screenBuff->drawString(buf, 4, y); y += LINE_H;
    } else {
        screenBuff->drawString("Not mounted", 4, y); y += LINE_H;
    }

    y += LINE_H;
    screenBuff->drawString("=== Tasks ===", 4, y); y += LINE_H;
    snprintf(buf, sizeof(buf), "Running: %u", uxTaskGetNumberOfTasks());
    screenBuff->drawString(buf, 4, y); y += LINE_H;
}

void SysInfoNonStaticApp::UpdateButtons(int button) {
    if (button == BUTTON_BACK) SystemCommon::Get().SetNextApp(&AppMenu::Get());
    if (button == BUTTON_HOME) SystemCommon::Get().SetNextApp(&MainMenu::Get());
    if (button == BUTTON_UP) { scroll.scrollY -= 24; scroll.clamp(); }
    if (button == BUTTON_DOWN) { scroll.scrollY += 24; scroll.clamp(); }
}

void SysInfoNonStaticApp::UpdateTouch(const TouchPoint* touches, int count) {
    if (count > 0) scroll.handleTouch(touches[0]);
}

void SysInfoNonStaticApp::CloseApp() {
    scroll.reset();
}

const uint16_t* SysInfoNonStaticApp::getIcon() { return sysinfo_icon; }
const uint16_t* SysInfoNonStaticApp::StaticIcon() { return sysinfo_icon; }
