#pragma once
#include "appTemplates/staticApp.h"
#include "nonStaticPrograms/phone.h"
#include "System/systemImages.h"


class PhoneAppNonStaticAppWrapper : public StaticApp {
public:
    static PhoneAppNonStaticAppWrapper &Get(std::string name = "Phone");

    PhoneAppNonStaticAppWrapper(const PhoneAppNonStaticAppWrapper &) = delete;
    PhoneAppNonStaticAppWrapper &operator=(const PhoneAppNonStaticAppWrapper &) = delete;

    void Setup() override;
    void Loop() override;
    void UpdateButtons(int button) override;
    void UpdateTouch(const TouchPoint* touches, int count) override;
    void Draw() override;
    const uint16_t* getIcon() override;
    
    std::string GetName() const override;
    void SetName(const std::string& name) override;

    void CloseApp() override;

    ~PhoneAppNonStaticAppWrapper();

private:
    PhoneAppNonStaticAppWrapper(std::string name);
    PhoneNonStaticApp* mApp = nullptr;
    std::string mName;
};
