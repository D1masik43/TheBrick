#pragma once

#include <memory>
#include <string>
#include <utility>

#include "appTemplates/staticApp.h"

template <typename TNonStaticApp>
class NonStaticAppWrapperBase : public StaticApp {
public:
    explicit NonStaticAppWrapperBase(std::string name)
        : StaticApp(name), mName(std::move(name)) {}

    void Setup() override {
        if (!mApp) {
            mApp.reset(new TNonStaticApp(mName));
            mApp->Setup();
        }
    }

    void Loop() override {
        if (mApp) {
            mApp->Loop();
        }
    }

    void UpdateButtons(int button) override {
        if (mApp) {
            mApp->UpdateButtons(button);
        }
    }

    void UpdateTouch(const TouchPoint* touches, int count) override {
        if (mApp) {
            mApp->UpdateTouch(touches, count);
        }
    }

    void Draw() override {
        if (mApp) {
            mApp->Draw();
        }
    }

    std::string GetName() const override {
        return mName;
    }

    void SetName(const std::string& name) override {
        mName = name;
        if (mApp) {
            mApp->SetName(name);
        }
    }

    void CloseApp() override {
        if (mApp) {
            mApp->CloseApp();
            mApp.reset();
        }
    }

    ~NonStaticAppWrapperBase() override {
        CloseApp();
    }

private:
    std::unique_ptr<TNonStaticApp> mApp;
    std::string mName;
};
