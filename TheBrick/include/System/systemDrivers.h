#pragma once
#include "appTemplates/staticApp.h"
#include <TFT_eSPI.h>

class SystemDrivers : public StaticApp
{
  public:
    static SystemDrivers &Get(std::string name = "SystemDrivers");

    SystemDrivers(const SystemDrivers &) = delete;
    SystemDrivers &operator=(const SystemDrivers &) = delete;

    void Loop() override;
    void Update(int button) override;
    void Setup() override;
    void Draw() override;

    const unsigned char *getIcon();

    static TFT_eSPI &GetTFT();

  private:
    SystemDrivers(std::string name);
    static TFT_eSPI tft;
};
