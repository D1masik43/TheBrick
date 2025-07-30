#include "staticPrograms/dummy_wrapper.h"

DummyAppNonStaticAppWrapper &DummyAppNonStaticAppWrapper::Get(std::string name) {
    static DummyAppNonStaticAppWrapper instance(name);
    return instance;
}

DummyAppNonStaticAppWrapper::DummyAppNonStaticAppWrapper(std::string name) : StaticApp(name) {

}

void DummyAppNonStaticAppWrapper::Setup() {
    if (!mApp) {
        mApp = new DummyNonStaticApp(mName);
        mApp->Setup();
    }
}

void DummyAppNonStaticAppWrapper::Loop() {
    if (mApp) mApp->Loop();
}

void DummyAppNonStaticAppWrapper::UpdateButtons(int button) {
    if (mApp) mApp->UpdateButtons(button);
}

void DummyAppNonStaticAppWrapper::UpdateTouch(const TouchPoint* touches, int count) {
    if (mApp) mApp->UpdateTouch(touches, count);
}

void DummyAppNonStaticAppWrapper::Draw() {
    if (mApp) mApp->Draw();
}

const uint16_t* DummyAppNonStaticAppWrapper::getIcon() {
    return &Icons::Dummy[0][0];
}

std::string DummyAppNonStaticAppWrapper::GetName() const {
    return mName;
}

void DummyAppNonStaticAppWrapper::SetName(const std::string& name) {
    mName = name;
}

void DummyAppNonStaticAppWrapper::CloseApp() {
    if (mApp) {
        delete mApp;
        mApp = nullptr;
    }
}

DummyAppNonStaticAppWrapper::~DummyAppNonStaticAppWrapper() {
    CloseApp();  // ensure cleanup
}
