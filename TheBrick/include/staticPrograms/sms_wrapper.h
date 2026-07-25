#pragma once
#include "appTemplates/staticApp.h"
#include "nonStaticPrograms/sms.h"
#include "System/systemImages.h"

class SmsAppNonStaticAppWrapper : public StaticApp {
public:
    static SmsAppNonStaticAppWrapper &Get(std::string name = "SMS");

    SmsAppNonStaticAppWrapper(const SmsAppNonStaticAppWrapper &) = delete;
    SmsAppNonStaticAppWrapper &operator=(const SmsAppNonStaticAppWrapper &) = delete;

    void Setup() override;
    void Loop() override;
    void UpdateButtons(int button) override;
    void UpdateTouch(const TouchPoint* touches, int count) override;
    void Draw() override;
    const uint16_t* getIcon() override;

    std::string GetName() const override;
    void SetName(const std::string& name) override;

    void CloseApp() override;
    ~SmsAppNonStaticAppWrapper();

private:
    SmsAppNonStaticAppWrapper(std::string name);
    SmsNonStaticApp* mApp = nullptr;
    std::string mName;
};
