#include "System/systemDrivers.h"

SystemDrivers &SystemDrivers::Get(std::string name) {
    static SystemDrivers instance(name);
    return instance;
}

TFT_eSPI &SystemDrivers::GetTFT() {
    static TFT_eSPI tft;
    return tft;
}

Adafruit_MCP23X17 &SystemDrivers::GetMCP() {
    static Adafruit_MCP23X17 mcp;
    return mcp;
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

    //  ====    TFT  ====
    TFT_eSPI *tft = &GetTFT();
    tft->begin();
    tft->setRotation(3);
    tft->invertDisplay(true);
    tft->fillScreen(TFT_BLACK);

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
}

void SystemDrivers::Draw() {

}


const unsigned char *SystemDrivers::getIcon() {
    return nullptr;
}


