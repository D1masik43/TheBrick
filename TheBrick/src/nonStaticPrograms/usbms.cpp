#include "nonStaticPrograms/usbms.h"
#include "staticPrograms/mainMenu.h"
#include "staticPrograms/appMenu.h"
#include "USB.h"
#include "USBMSC.h"
#include "sdmmc_cmd.h"
#include "SD_MMC.h"

static USBMSC msc;
static bool mscInitialized = false;

struct SDCardHelper : public fs::SDMMCFS {
    sdmmc_card_t* card() { return _card; }
};

static sdmmc_card_t* getSDCard() {
    return reinterpret_cast<SDCardHelper*>(&SD_MMC)->card();
}

static int32_t mscRead(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize) {
    sdmmc_card_t* card = getSDCard();
    if (!card) return -1;
    if (sdmmc_read_sectors(card, buffer, lba, bufsize / 512) == ESP_OK)
        return bufsize;
    return -1;
}

static int32_t mscWrite(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize) {
    sdmmc_card_t* card = getSDCard();
    if (!card) return -1;
    if (sdmmc_write_sectors(card, buffer, lba, bufsize / 512) == ESP_OK)
        return bufsize;
    return -1;
}

static void initMSCOnce() {
    if (mscInitialized) return;
    mscInitialized = true;

    msc.vendorID("TheBrick");
    msc.productID("SD Card");
    msc.productRevision("1.0");
    msc.onRead(mscRead);
    msc.onWrite(mscWrite);
    msc.mediaPresent(false);

    sdmmc_card_t* card = getSDCard();
    if (card) {
        msc.begin(card->csd.capacity, card->csd.sector_size);
    }

    USB.begin();
}

static const uint16_t usbms_icon[256] = {0};

UsbmsNonStaticApp::UsbmsNonStaticApp(const std::string& name)
    : NonStaticApp(name) {}

void UsbmsNonStaticApp::Setup() {
    screenBuff = &SystemDrivers::Get().GetScreenBuff();
    connected = false;

    connectBtn.SetColors(TFT_WHITE, 0x0400, TFT_GREEN);
    disconnectBtn.SetColors(TFT_WHITE, 0x4000, TFT_RED);

    if (sdAvailable) initMSCOnce();
}

void UsbmsNonStaticApp::startMSC() {
    if (!sdAvailable) return;

    sdAvailable = false;
    msc.mediaPresent(true);
    connected = true;
}

void UsbmsNonStaticApp::stopMSC() {
    msc.mediaPresent(false);
    connected = false;

    // remount to flush stale FAT caches after USB writes
    SD_MMC.end();
    SD_MMC.setPins(39, 38, 40);
    if (SD_MMC.begin("/sdcard", true)) {
        sdAvailable = true;
        sdmmc_card_t* card = getSDCard();
        if (card) msc.begin(card->csd.capacity, card->csd.sector_size);
    }
}

void UsbmsNonStaticApp::Loop() {
    Draw();
}

void UsbmsNonStaticApp::Draw() {
    screenBuff->fillScreen(TFT_BLACK);
    screenBuff->setTextSize(2);
    screenBuff->setTextDatum(MC_DATUM);
    screenBuff->setTextColor(TFT_WHITE);
    screenBuff->drawString("USB Storage", 120, 40);

    screenBuff->setTextSize(1);

    if (!sdAvailable && !connected) {
        screenBuff->setTextColor(TFT_RED);
        screenBuff->drawString("SD card not available", 120, 160);
    } else if (connected) {
        screenBuff->setTextColor(TFT_GREEN);
        screenBuff->drawString("Connected to PC", 120, 100);
        screenBuff->setTextColor(TFT_DARKGREY);
        screenBuff->drawString("Eject from PC before", 120, 220);
        screenBuff->drawString("disconnecting", 120, 235);
        disconnectBtn.Draw(*screenBuff);
    } else {
        screenBuff->setTextColor(TFT_DARKGREY);
        screenBuff->drawString("Plug USB cable to PC", 120, 100);
        connectBtn.Draw(*screenBuff);
    }

    screenBuff->setTextDatum(TL_DATUM);
}

void UsbmsNonStaticApp::UpdateButtons(int button) {
    if (button == BUTTON_BACK) {
        if (connected) stopMSC();
        SystemCommon::Get().SetNextApp(&AppMenu::Get());
    }
    if (button == BUTTON_HOME) {
        if (connected) stopMSC();
        SystemCommon::Get().SetNextApp(&MainMenu::Get());
    }
    if (button == BUTTON_IN) {
        if (connected) stopMSC();
        else startMSC();
    }
}

void UsbmsNonStaticApp::UpdateTouch(const TouchPoint* touches, int count) {
    if (count <= 0) return;
    const TouchPoint& tp = touches[0];
    if (tp.type != TAP) return;

    if (connected) {
        if (disconnectBtn.IsPressed(tp) != -1) stopMSC();
    } else {
        if (connectBtn.IsPressed(tp) != -1) startMSC();
    }
}

void UsbmsNonStaticApp::CloseApp() {
    if (connected) stopMSC();
}

const uint16_t* UsbmsNonStaticApp::getIcon() { return usbms_icon; }
const uint16_t* UsbmsNonStaticApp::StaticIcon() { return usbms_icon; }
