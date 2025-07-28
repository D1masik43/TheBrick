#pragma once
#include "appTemplates/staticApp.h"
#include "System/systemDrivers.h"
#include <TFT_eSPI.h>
#include "System/systemStructs.h"
#include "System/systemImages.h"
#include "System/systemCommon.h"

class AppMenu : public StaticApp
{
  public:
    static AppMenu &Get(std::string name = "MainMenu");

    AppMenu(const AppMenu &) = delete;
    AppMenu &operator=(const AppMenu &) = delete;

    void Loop() override;
    void UpdateButtons(int button) override;
    void UpdateTouch(const TouchPoint* touches, int count) override;
    void Setup() override;
    void Draw() override;

    const unsigned char *getIcon();

  private:
    AppMenu(std::string name);

    void DrawBlurredPatch(int x0, int y0, int w, int h);

    TFT_eSprite *screenBuff;
};
