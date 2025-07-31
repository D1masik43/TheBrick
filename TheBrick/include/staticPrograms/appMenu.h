#pragma once
#include "appTemplates/staticApp.h"
#include "System/systemDrivers.h"
#include <TFT_eSPI.h>
#include "System/systemStructs.h"
#include "System/systemImages.h"
#include "System/systemCommon.h"
#include "staticPrograms/allStaticPrograms.h"

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

    const uint16_t *getIcon();

  private:
    AppMenu(std::string name);

    void DrawBlurredPatch(int x0, int y0, int w, int h);

    TFT_eSprite *screenBuff;

    AppBase *appList[6][3] = {
      {&DummyAppNonStaticAppWrapper::Get(),&DummyAppNonStaticAppWrapper::Get(),&DummyAppNonStaticAppWrapper::Get()},
      {&DummyAppNonStaticAppWrapper::Get(),&DummyAppNonStaticAppWrapper::Get(),&DummyAppNonStaticAppWrapper::Get()},
      {&DummyAppNonStaticAppWrapper::Get(),&DummyAppNonStaticAppWrapper::Get(),&DummyAppNonStaticAppWrapper::Get()},
      {&DummyAppNonStaticAppWrapper::Get(),&DummyAppNonStaticAppWrapper::Get(),&DummyAppNonStaticAppWrapper::Get()},
      {&DummyAppNonStaticAppWrapper::Get(),&DummyAppNonStaticAppWrapper::Get(),&DummyAppNonStaticAppWrapper::Get()},
      {&DummyAppNonStaticAppWrapper::Get(),&DummyAppNonStaticAppWrapper::Get(),&DummyAppNonStaticAppWrapper::Get()},
    }; 

    int startPoint = 36;
    int iconSize = 48;
    int paddingIcons = 24 + iconSize;
    int paddingX = 24;

    int totalOffsetY = 0;
    int slideStartY = 0;
    int lastSlideY = 0;
    bool isSliding = false;


};
