#include "System/systemDrivers.h"

SemaphoreHandle_t i2cMutex = nullptr;
SemaphoreHandle_t sim800Mutex = nullptr;
bool mcpAvailable = false;
bool rtcAvailable = false;
bool sdAvailable = false;

SystemDrivers &SystemDrivers::Get(std::string name) {
    static SystemDrivers instance(name);
    return instance;
}

TFT_eSPI &SystemDrivers::GetTFT() {
    static TFT_eSPI tft;
    return tft;
}

TFT_eSprite &SystemDrivers::GetScreenBuff() {
    static TFT_eSprite screenBuffer = TFT_eSprite(&GetTFT()); 
    return screenBuffer;
}

Adafruit_MCP23X17 &SystemDrivers::GetMCP() {
    static Adafruit_MCP23X17 mcp;
    return mcp;
}

HardwareSerial &SystemDrivers::GetSim800() {
    static HardwareSerial sim800(1);
    return sim800;
}

arduino::ft6336<SCREEN_WIDTH, SCREEN_HEIGHT> &SystemDrivers::GetTouch() {
    static arduino::ft6336<SCREEN_WIDTH, SCREEN_HEIGHT> touch;
    return touch;
}

RTC_DS3231 &SystemDrivers::GetRTC() {
    static RTC_DS3231 rtc;
    return rtc;
}

Audio &SystemDrivers::GetAudio() {
    static Audio audio;
    return audio;
}

Adafruit_INA219 &SystemDrivers::GetINA219() {
    static Adafruit_INA219 ina219;
    return ina219;
}

SystemDrivers::SystemDrivers(std::string name) : StaticApp(name) {

}

void SystemDrivers::Loop() {

}

void SystemDrivers::UpdateButtons(int button) {

}

void SystemDrivers::UpdateTouch(const TouchPoint* touches, int count) {

}

void SystemDrivers::Setup() {

    //  ====    Serial  ====
    Serial.begin(115200);

    //  ====    I2C  ====
    i2cMutex = xSemaphoreCreateMutex();
    sim800Mutex = xSemaphoreCreateMutex();
    Wire.begin(45, 48);
    Wire.setClock(400000);

    Wire.setTimeOut(10);
    Serial.println("I2C scan:");
    for (uint8_t addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            Serial.printf("  0x%02X found\n", addr);
        }
    }
    Serial.println("I2C scan done");
    Wire.setTimeOut(50);

    //  ====    INA219  ====
    Adafruit_INA219 &ina219 = GetINA219();
    if (ina219.begin(&Wire)) {
        Serial.println("INA219 good");
    } else {
        Serial.println("INA219 NOT good");
    }

    //  ====    TFT  ====
    TFT_eSPI *tft = &GetTFT();
    tft->begin();
    tft->setRotation(0);
    tft->invertDisplay(true);
    tft->fillScreen(TFT_BLACK);

    TFT_eSprite *screenBuffer = &GetScreenBuff(); 
    screenBuffer->setColorDepth(16);
    screenBuffer->setAttribute(PSRAM_ENABLE, true);
    screenBuffer->createSprite(240, 320);

    //  ====    MCP  ====
    Adafruit_MCP23X17 &mcp = GetMCP();
    if(mcp.begin_I2C(0x20, &Wire))
    {
        Serial.println("MCP good");
        mcpAvailable = true;
    }
    else
    {
        Serial.println("MCP NOT good");
        mcpAvailable = false;
    }

    //  ====    ButtonHandler  ====
    buttonEventQueue = xQueueCreate(10, sizeof(int));
    if (mcpAvailable) {
        xTaskCreatePinnedToCore(buttonTask, "ButtonTask", 4096, NULL, 1, NULL, 0);
    } else {
        Serial.println("ButtonTask skipped - MCP not available");
    }

     //  ====    FT6336  ====

    arduino::ft6336<SCREEN_WIDTH, SCREEN_HEIGHT>  &touch = GetTouch();
    if (!touch.initialize()) {
        Serial.println("Touch not good");
    }
    else{
        Serial.println("Touch good");
    }

    //  ====    TouchHandler  ====

    touchEventQueue = xQueueCreate(4, sizeof(TouchPoint[2]));  // Room for 4 touch events
    xTaskCreatePinnedToCore(touchTask, "TouchTask", 4096, NULL, 1, NULL, 0);

    // ==== SIM800 UART ====
    HardwareSerial &sim800 = GetSim800();
    sim800.begin(9600, SERIAL_8N1, 0, 1); 
    Serial.println("SIM800 UART started");
    sim800.println("AT+CPIN=\"9205\"");
    sim800.println("AT+CLVL=100");
    
    // ==== DS3231 ====
    RTC_DS3231 &rtc = GetRTC();
    if (!rtc.begin()) {
        Serial.println("RTC NOT found");
        rtcAvailable = false;
    } else {
        Serial.println("RTC found");
        rtcAvailable = true;
        if (rtc.lostPower()) {
            Serial.println("RTC lost power, setting time!");
            rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        }
    }

    // ==== eMMC SD ====
    SD_MMC.setPins(39, 38, 40);
    if (!SD_MMC.begin("/sdcard", true)) {
        Serial.println("SD Card Mount Failed");
        sdAvailable = false;
    } else {
        sdAvailable = true;
        Serial.println("SD Card initialized.");
        uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
        Serial.printf("SD Card Size: %llu MB\n", cardSize);
    }

}

void SystemDrivers::Draw() {

}


const uint16_t *SystemDrivers::getIcon() {
    return nullptr;
}


