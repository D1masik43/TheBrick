#pragma once
#include "appTemplates/staticApp.h"
#include "nonStaticPrograms/wifi_settings.h"

class WifiAppNonStaticAppWrapper : public StaticApp {
public:
    static WifiAppNonStaticAppWrapper &Get(std::string name = "WiFi");

    WifiAppNonStaticAppWrapper(const WifiAppNonStaticAppWrapper &) = delete;
    WifiAppNonStaticAppWrapper &operator=(const WifiAppNonStaticAppWrapper &) = delete;

    void Setup() override;
    void Loop() override;
    void UpdateButtons(int button) override;
    void UpdateTouch(const TouchPoint* touches, int count) override;
    void Draw() override;
    const uint16_t* getIcon() override;

    std::string GetName() const override;
    void SetName(const std::string& name) override;

    void CloseApp() override;
    ~WifiAppNonStaticAppWrapper();

private:
    WifiAppNonStaticAppWrapper(std::string name);
    WifiSettingsApp* mApp = nullptr;
    std::string mName;
};
