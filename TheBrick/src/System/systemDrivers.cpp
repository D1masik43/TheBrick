#include "System/systemDrivers.h"

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

arduino::ft6336<SCREEN_WIDTH, SCREEN_HEIGHT> &SystemDrivers::GetTouch() {
    static arduino::ft6336<SCREEN_WIDTH, SCREEN_HEIGHT> touch;
    return touch;
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
    Wire.begin(45, 48);  // SDA = 45, SCL = 48
    Wire.setClock(400000);

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
    }
    else
    {
        Serial.println("MCP NOT good");
    }

    //  ====    ButtonHandler  ====
    buttonEventQueue = xQueueCreate(10, sizeof(int));
    xTaskCreatePinnedToCore(buttonTask, "ButtonTask", 4096, NULL, 1, NULL, 0);

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
}

void SystemDrivers::Draw() {

}


const uint16_t *SystemDrivers::getIcon() {
    return nullptr;
}


