#include "staticPrograms/wifi_wrapper.h"

WifiAppNonStaticAppWrapper &WifiAppNonStaticAppWrapper::Get(std::string name) {
    static WifiAppNonStaticAppWrapper instance(name);
    return instance;
}

WifiAppNonStaticAppWrapper::WifiAppNonStaticAppWrapper(std::string name) : StaticApp(name) {}

void WifiAppNonStaticAppWrapper::Setup() {
    if (!mApp) {
        mApp = new WifiSettingsApp(mName);
        mApp->Setup();
    }
}

void WifiAppNonStaticAppWrapper::Loop() { if (mApp) mApp->Loop(); }
void WifiAppNonStaticAppWrapper::UpdateButtons(int button) { if (mApp) mApp->UpdateButtons(button); }
void WifiAppNonStaticAppWrapper::UpdateTouch(const TouchPoint* touches, int count) { if (mApp) mApp->UpdateTouch(touches, count); }
void WifiAppNonStaticAppWrapper::Draw() { if (mApp) mApp->Draw(); }

const uint16_t* WifiAppNonStaticAppWrapper::getIcon() { return WifiSettingsApp::StaticIcon(); }

std::string WifiAppNonStaticAppWrapper::GetName() const { return mName; }
void WifiAppNonStaticAppWrapper::SetName(const std::string& name) { mName = name; }

void WifiAppNonStaticAppWrapper::CloseApp() {
    if (mApp) {
        mApp->CloseApp();
        delete mApp;
        mApp = nullptr;
    }
}

WifiAppNonStaticAppWrapper::~WifiAppNonStaticAppWrapper() { CloseApp(); }
