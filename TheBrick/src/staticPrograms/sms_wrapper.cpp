#include "staticPrograms/sms_wrapper.h"

SmsAppNonStaticAppWrapper &SmsAppNonStaticAppWrapper::Get(std::string name) {
    static SmsAppNonStaticAppWrapper instance(name);
    return instance;
}

SmsAppNonStaticAppWrapper::SmsAppNonStaticAppWrapper(std::string name) : StaticApp(name) {}

void SmsAppNonStaticAppWrapper::Setup() {
    if (!mApp) {
        mApp = new SmsNonStaticApp(mName);
        mApp->Setup();
    }
}

void SmsAppNonStaticAppWrapper::Loop() { if (mApp) mApp->Loop(); }
void SmsAppNonStaticAppWrapper::UpdateButtons(int button) { if (mApp) mApp->UpdateButtons(button); }
void SmsAppNonStaticAppWrapper::UpdateTouch(const TouchPoint* touches, int count) { if (mApp) mApp->UpdateTouch(touches, count); }
void SmsAppNonStaticAppWrapper::Draw() { if (mApp) mApp->Draw(); }

const uint16_t* SmsAppNonStaticAppWrapper::getIcon() {
    return &Icons::SMS[0][0];
}

std::string SmsAppNonStaticAppWrapper::GetName() const { return mName; }
void SmsAppNonStaticAppWrapper::SetName(const std::string& name) { mName = name; }

void SmsAppNonStaticAppWrapper::CloseApp() {
    if (mApp) {
        mApp->CloseApp();
        delete mApp;
        mApp = nullptr;
    }
}

SmsAppNonStaticAppWrapper::~SmsAppNonStaticAppWrapper() { CloseApp(); }
