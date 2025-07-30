#pragma once
#include "appTemplates/staticApp.h"
#include "nonStaticPrograms/dummy.h"
#include "System/systemImages.h"


class DummyAppNonStaticAppWrapper : public StaticApp {
public:
    static DummyAppNonStaticAppWrapper &Get(std::string name = "MainMenu");

    DummyAppNonStaticAppWrapper(const DummyAppNonStaticAppWrapper &) = delete;
    DummyAppNonStaticAppWrapper &operator=(const DummyAppNonStaticAppWrapper &) = delete;

    void Setup() override;
    void Loop() override;
    void UpdateButtons(int button) override;
    void UpdateTouch(const TouchPoint* touches, int count) override;
    void Draw() override;
    const uint16_t* getIcon() override;
    
    std::string GetName() const override;
    void SetName(const std::string& name) override;

    void CloseApp() override;

    ~DummyAppNonStaticAppWrapper();

private:
    DummyAppNonStaticAppWrapper(std::string name);
    DummyNonStaticApp* mApp = nullptr;
    std::string mName;
};
