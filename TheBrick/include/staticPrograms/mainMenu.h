#pragma once
#include "appTemplates/staticApp.h"
#include "System/systemDrivers.h"
#include <TFT_eSPI.h>
#include "System/systemStructs.h"
#include "System/systemImages.h"
#include "System/systemCommon.h"
#include "staticPrograms/appMenu.h"
#include <esp_heap_caps.h>

class MainMenu : public StaticApp
{
  public:
    static MainMenu &Get(std::string name = "MainMenu");

    MainMenu(const MainMenu &) = delete;
    MainMenu &operator=(const MainMenu &) = delete;

    void Loop() override;
    void UpdateButtons(int button) override;
    void UpdateTouch(const TouchPoint* touches, int count) override;
    void Setup() override;
    void Draw() override;

    const unsigned char *getIcon();

  private:
    MainMenu(std::string name);

    void DrawBlurredPatch(int x0, int y0, int w, int h, int blurIntensity);

    TFT_eSprite *screenBuff;
};
