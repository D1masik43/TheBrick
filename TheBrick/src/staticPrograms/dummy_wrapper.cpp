#include "staticPrograms/dummy_wrapper.h"

MyNonStaticAppWrapper &MyNonStaticAppWrapper::Get(std::string name) {
    static MyNonStaticAppWrapper instance(name);
    return instance;
}

MyNonStaticAppWrapper::MyNonStaticAppWrapper(std::string name) : StaticApp(name) {

}

void MyNonStaticAppWrapper::Setup() {
    if (!mApp) {
        mApp = new DummyNonStaticApp(mName);
        mApp->Setup();
    }
}

void MyNonStaticAppWrapper::Loop() {
    if (mApp) mApp->Loop();
}

void MyNonStaticAppWrapper::UpdateButtons(int button) {
    if (mApp) mApp->UpdateButtons(button);
}

void MyNonStaticAppWrapper::UpdateTouch(const TouchPoint* touches, int count) {
    if (mApp) mApp->UpdateTouch(touches, count);
}

void MyNonStaticAppWrapper::Draw() {
    if (mApp) mApp->Draw();
}

const uint16_t* MyNonStaticAppWrapper::getIcon() {
    return &Icons::Dummy[0][0];
}

std::string MyNonStaticAppWrapper::GetName() const {
    return mName;
}

void MyNonStaticAppWrapper::SetName(const std::string& name) {
    mName = name;
}

void MyNonStaticAppWrapper::CloseApp() {
    if (mApp) {
        delete mApp;
        mApp = nullptr;
    }
}

MyNonStaticAppWrapper::~MyNonStaticAppWrapper() {
    CloseApp();  // ensure cleanup
}
