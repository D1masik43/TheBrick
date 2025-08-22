#include "staticPrograms/phone_wrapper.h"

PhoneAppNonStaticAppWrapper &PhoneAppNonStaticAppWrapper::Get(std::string name) {
    static PhoneAppNonStaticAppWrapper instance(name);
    return instance;
}

PhoneAppNonStaticAppWrapper::PhoneAppNonStaticAppWrapper(std::string name) : StaticApp(name) {

}

void PhoneAppNonStaticAppWrapper::Setup() {
    if (!mApp) {
        mApp = new PhoneNonStaticApp(mName);
        mApp->Setup();
    }
}

void PhoneAppNonStaticAppWrapper::Loop() {
    if (mApp) mApp->Loop();
}

void PhoneAppNonStaticAppWrapper::UpdateButtons(int button) {
    if (mApp) mApp->UpdateButtons(button);
}

void PhoneAppNonStaticAppWrapper::UpdateTouch(const TouchPoint* touches, int count) {
    if (mApp) mApp->UpdateTouch(touches, count);
}

void PhoneAppNonStaticAppWrapper::Draw() {
    if (mApp) mApp->Draw();
}

const uint16_t* PhoneAppNonStaticAppWrapper::getIcon() {
    return &Icons::Phone[0][0];
}

std::string PhoneAppNonStaticAppWrapper::GetName() const {
    return mName;
}

void PhoneAppNonStaticAppWrapper::SetName(const std::string& name) {
    mName = name;
}

void PhoneAppNonStaticAppWrapper::CloseApp() {
    if (mApp) {
        mApp->CloseApp();
        delete mApp;
        mApp = nullptr;
    }
}

PhoneAppNonStaticAppWrapper::~PhoneAppNonStaticAppWrapper() {
    CloseApp();
}
