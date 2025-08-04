#pragma once
#include "appTemplates/staticApp.h"
#include <TFT_eSPI.h>

class Mainmenu;

class SystemCommon : public StaticApp
{
  public:
    static SystemCommon &Get(std::string name = "MainMenu");

    SystemCommon(const SystemCommon &) = delete;
    SystemCommon &operator=(const SystemCommon &) = delete;

    void Loop() override;
    void UpdateButtons(int button) override;
    void UpdateTouch(const TouchPoint* touches, int count) override;
    void Setup() override;
    void Draw() override;

    const uint16_t *getIcon();

    AppBase* GetCurrentApp();

    void SetNextApp(AppBase* app);
    void ProcessAppSwitch();

  private:
    SystemCommon(std::string name);

    static AppBase* currentApp;
    static AppBase* nextApp;
};
