#include "staticPrograms/sysinfo_wrapper.h"

SysInfoAppNonStaticAppWrapper &SysInfoAppNonStaticAppWrapper::Get(std::string name) {
    static SysInfoAppNonStaticAppWrapper instance(name);
    return instance;
}

SysInfoAppNonStaticAppWrapper::SysInfoAppNonStaticAppWrapper(std::string name) : StaticApp(name) {
}

void SysInfoAppNonStaticAppWrapper::Setup() {
    if (!mApp) {
        mApp = new SysInfoNonStaticApp(mName);
        mApp->Setup();
    }
}

void SysInfoAppNonStaticAppWrapper::Loop() {
    if (mApp) mApp->Loop();
}

void SysInfoAppNonStaticAppWrapper::UpdateButtons(int button) {
    if (mApp) mApp->UpdateButtons(button);
}

void SysInfoAppNonStaticAppWrapper::UpdateTouch(const TouchPoint* touches, int count) {
    if (mApp) mApp->UpdateTouch(touches, count);
}

void SysInfoAppNonStaticAppWrapper::Draw() {
    if (mApp) mApp->Draw();
}

const uint16_t* SysInfoAppNonStaticAppWrapper::getIcon() {
    return &Icons::Settings[0][0];
}

std::string SysInfoAppNonStaticAppWrapper::GetName() const {
    return mName;
}

void SysInfoAppNonStaticAppWrapper::SetName(const std::string& name) {
    mName = name;
}

void SysInfoAppNonStaticAppWrapper::CloseApp() {
    if (mApp) {
        mApp->CloseApp();
        delete mApp;
        mApp = nullptr;
    }
}

SysInfoAppNonStaticAppWrapper::~SysInfoAppNonStaticAppWrapper() {
    CloseApp();
}
