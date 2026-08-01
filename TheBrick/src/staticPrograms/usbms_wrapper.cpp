#include "staticPrograms/usbms_wrapper.h"

UsbmsAppNonStaticAppWrapper &UsbmsAppNonStaticAppWrapper::Get(std::string name) {
    static UsbmsAppNonStaticAppWrapper instance(name);
    return instance;
}

UsbmsAppNonStaticAppWrapper::UsbmsAppNonStaticAppWrapper(std::string name) : StaticApp(name) {}

void UsbmsAppNonStaticAppWrapper::Setup() {
    if (!mApp) {
        mApp = new UsbmsNonStaticApp(mName);
        mApp->Setup();
    }
}

void UsbmsAppNonStaticAppWrapper::Loop() {
    if (mApp) mApp->Loop();
}

void UsbmsAppNonStaticAppWrapper::UpdateButtons(int button) {
    if (mApp) mApp->UpdateButtons(button);
}

void UsbmsAppNonStaticAppWrapper::UpdateTouch(const TouchPoint* touches, int count) {
    if (mApp) mApp->UpdateTouch(touches, count);
}

void UsbmsAppNonStaticAppWrapper::Draw() {
    if (mApp) mApp->Draw();
}

const uint16_t* UsbmsAppNonStaticAppWrapper::getIcon() {
    return UsbmsNonStaticApp::StaticIcon();
}

std::string UsbmsAppNonStaticAppWrapper::GetName() const {
    return mName;
}

void UsbmsAppNonStaticAppWrapper::SetName(const std::string& name) {
    mName = name;
}

void UsbmsAppNonStaticAppWrapper::CloseApp() {
    if (mApp) {
        mApp->CloseApp();
        delete mApp;
        mApp = nullptr;
    }
}

UsbmsAppNonStaticAppWrapper::~UsbmsAppNonStaticAppWrapper() {
    CloseApp();
}
