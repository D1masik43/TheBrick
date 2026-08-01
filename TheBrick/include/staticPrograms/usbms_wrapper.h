#pragma once
#include "appTemplates/staticApp.h"
#include "nonStaticPrograms/usbms.h"
#include "System/systemImages.h"

class UsbmsAppNonStaticAppWrapper : public StaticApp {
public:
    static UsbmsAppNonStaticAppWrapper &Get(std::string name = "USB");

    UsbmsAppNonStaticAppWrapper(const UsbmsAppNonStaticAppWrapper &) = delete;
    UsbmsAppNonStaticAppWrapper &operator=(const UsbmsAppNonStaticAppWrapper &) = delete;

    void Setup() override;
    void Loop() override;
    void UpdateButtons(int button) override;
    void UpdateTouch(const TouchPoint* touches, int count) override;
    void Draw() override;
    const uint16_t* getIcon() override;

    std::string GetName() const override;
    void SetName(const std::string& name) override;

    void CloseApp() override;

    ~UsbmsAppNonStaticAppWrapper();

private:
    UsbmsAppNonStaticAppWrapper(std::string name);
    UsbmsNonStaticApp* mApp = nullptr;
    std::string mName;
};
