#include "nonStaticPrograms/sysinfo.h"
#include "staticPrograms/mainMenu.h"

static const uint16_t sysinfo_icon[256] = {0};

SysInfoNonStaticApp::SysInfoNonStaticApp(const std::string& name)
    : NonStaticApp(name) {}

void SysInfoNonStaticApp::Setup() {
    screenBuff = &SystemDrivers::Get().GetScreenBuff();
}

void SysInfoNonStaticApp::Loop() {
    if (millis() - lastUpdate > 500) {
        lastUpdate = millis();
        if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(20)) == pdTRUE) {
            auto &ina = SystemDrivers::Get().GetINA219();
            inaVoltage = ina.getBusVoltage_V();
            inaCurrent = ina.getCurrent_mA();
            inaPower = ina.getPower_mW();
            inaShunt = ina.getShuntVoltage_mV();
            xSemaphoreGive(i2cMutex);
        }
    }
    Draw();
}

void SysInfoNonStaticApp::Draw() {
    screenBuff->fillScreen(TFT_BLACK);
    screenBuff->setTextColor(TFT_WHITE);
    screenBuff->setTextSize(1);

    int y = 4 - scrollY;
    int lineH = 12;
    char buf[64];

    screenBuff->drawString("=== INA219 ===", 4, y); y += lineH;
    snprintf(buf, sizeof(buf), "Bus:    %.2f V", inaVoltage);
    screenBuff->drawString(buf, 4, y); y += lineH;
    snprintf(buf, sizeof(buf), "Shunt:  %.2f mV", inaShunt);
    screenBuff->drawString(buf, 4, y); y += lineH;
    snprintf(buf, sizeof(buf), "Current:%.1f mA", inaCurrent);
    screenBuff->drawString(buf, 4, y); y += lineH;
    snprintf(buf, sizeof(buf), "Power:  %.1f mW", inaPower);
    screenBuff->drawString(buf, 4, y); y += lineH;

    y += lineH;
    screenBuff->drawString("=== Memory ===", 4, y); y += lineH;
    snprintf(buf, sizeof(buf), "Heap free: %u", esp_get_free_heap_size());
    screenBuff->drawString(buf, 4, y); y += lineH;
    snprintf(buf, sizeof(buf), "Heap min:  %u", esp_get_minimum_free_heap_size());
    screenBuff->drawString(buf, 4, y); y += lineH;
    snprintf(buf, sizeof(buf), "PSRAM free:%u", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    screenBuff->drawString(buf, 4, y); y += lineH;

    y += lineH;
    screenBuff->drawString("=== CPU ===", 4, y); y += lineH;
    snprintf(buf, sizeof(buf), "Freq: %u MHz", getCpuFrequencyMhz());
    screenBuff->drawString(buf, 4, y); y += lineH;
    snprintf(buf, sizeof(buf), "Chip: %s rev %d", ESP.getChipModel(), ESP.getChipRevision());
    screenBuff->drawString(buf, 4, y); y += lineH;
    snprintf(buf, sizeof(buf), "Flash: %u MB", ESP.getFlashChipSize() / 1024 / 1024);
    screenBuff->drawString(buf, 4, y); y += lineH;

    y += lineH;
    screenBuff->drawString("=== I2C ===", 4, y); y += lineH;
    snprintf(buf, sizeof(buf), "MCP:  %s", mcpAvailable ? "OK" : "FAIL");
    screenBuff->drawString(buf, 4, y); y += lineH;
    snprintf(buf, sizeof(buf), "RTC:  %s", rtcAvailable ? "OK" : "FAIL");
    screenBuff->drawString(buf, 4, y); y += lineH;

    y += lineH;
    screenBuff->drawString("=== SD ===", 4, y); y += lineH;
    snprintf(buf, sizeof(buf), "Size: %llu MB", SD_MMC.cardSize() / 1024 / 1024);
    screenBuff->drawString(buf, 4, y); y += lineH;
    snprintf(buf, sizeof(buf), "Used: %llu MB", SD_MMC.usedBytes() / 1024 / 1024);
    screenBuff->drawString(buf, 4, y); y += lineH;

    y += lineH;
    screenBuff->drawString("=== Tasks ===", 4, y); y += lineH;
    snprintf(buf, sizeof(buf), "Running: %u", uxTaskGetNumberOfTasks());
    screenBuff->drawString(buf, 4, y); y += lineH;

    SystemDrivers::Get().GetTFT().pushImage(0, 0, 240, 320, (uint16_t*)screenBuff->getPointer());
}

void SysInfoNonStaticApp::UpdateButtons(int button) {
    if (button == BUTTON_BACK) SystemCommon::Get().SetNextApp(&AppMenu::Get());
    if (button == BUTTON_HOME) SystemCommon::Get().SetNextApp(&MainMenu::Get());
    if (button == BUTTON_UP) scrollY = max(0, scrollY - 24);
    if (button == BUTTON_DOWN) scrollY += 24;
}

void SysInfoNonStaticApp::UpdateTouch(const TouchPoint* touches, int count) {
}

void SysInfoNonStaticApp::CloseApp() {
    scrollY = 0;
}

const uint16_t* SysInfoNonStaticApp::getIcon() { return sysinfo_icon; }
const uint16_t* SysInfoNonStaticApp::StaticIcon() { return sysinfo_icon; }
