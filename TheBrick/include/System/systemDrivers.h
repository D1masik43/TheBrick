#pragma once
#include "appTemplates/staticApp.h"
#include <TFT_eSPI.h>
#include <Adafruit_MCP23X17.h>
#include "System/systemStructs.h"
#include "System/buttonHandler.h"
#include "System/systemDefines.h"

class SystemDrivers : public StaticApp
{
  public:
    static SystemDrivers &Get(std::string name = "SystemDrivers");

    SystemDrivers(const SystemDrivers &) = delete;
    SystemDrivers &operator=(const SystemDrivers &) = delete;

    void Loop() override;
    void UpdateButtons(int button) override;
    void UpdateTouch(const TouchPoint* touches, int count) override;
    void Setup() override;
    void Draw() override;

    const unsigned char *getIcon();

    static TFT_eSPI &GetTFT();
    static Adafruit_MCP23X17 &GetMCP();
    static arduino::ft6336<SCREEN_WIDTH, SCREEN_HEIGHT> &GetTouch();
    static TFT_eSprite &GetScreenBuff();

  private:
    SystemDrivers(std::string name);
    
};