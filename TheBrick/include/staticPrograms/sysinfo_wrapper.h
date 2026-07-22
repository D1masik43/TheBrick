#pragma once
#include "appTemplates/staticApp.h"
#include "nonStaticPrograms/sysinfo.h"
#include "System/systemImages.h"

class SysInfoAppNonStaticAppWrapper : public StaticApp {
public:
    static SysInfoAppNonStaticAppWrapper &Get(std::string name = "SysInfo");

    SysInfoAppNonStaticAppWrapper(const SysInfoAppNonStaticAppWrapper &) = delete;
    SysInfoAppNonStaticAppWrapper &operator=(const SysInfoAppNonStaticAppWrapper &) = delete;

    void Setup() override;
    void Loop() override;
    void UpdateButtons(int button) override;
    void UpdateTouch(const TouchPoint* touches, int count) override;
    void Draw() override;
    const uint16_t* getIcon() override;

    std::string GetName() const override;
    void SetName(const std::string& name) override;

    void CloseApp() override;

    ~SysInfoAppNonStaticAppWrapper();

private:
    SysInfoAppNonStaticAppWrapper(std::string name);
    SysInfoNonStaticApp* mApp = nullptr;
    std::string mName;
};
