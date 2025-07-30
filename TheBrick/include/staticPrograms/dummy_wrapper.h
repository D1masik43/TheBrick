#pragma once
#include "appTemplates/staticApp.h"
#include "nonStaticPrograms/dummy.h"
#include "System/systemImages.h"


class MyNonStaticAppWrapper : public StaticApp {
public:
    static MyNonStaticAppWrapper &Get(std::string name = "MainMenu");

    MyNonStaticAppWrapper(const MyNonStaticAppWrapper &) = delete;
    MyNonStaticAppWrapper &operator=(const MyNonStaticAppWrapper &) = delete;

    void Setup() override;
    void Loop() override;
    void UpdateButtons(int button) override;
    void UpdateTouch(const TouchPoint* touches, int count) override;
    void Draw() override;
    const uint16_t* getIcon() override;
    
    std::string GetName() const override;
    void SetName(const std::string& name) override;

    void CloseApp() override;

    ~MyNonStaticAppWrapper();

private:
    MyNonStaticAppWrapper(std::string name);
    DummyNonStaticApp* mApp = nullptr;
    std::string mName;
};
