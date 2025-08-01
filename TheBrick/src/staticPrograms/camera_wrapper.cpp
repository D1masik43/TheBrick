#include "staticPrograms/camera_wrapper.h"

CameraAppNonStaticAppWrapper &CameraAppNonStaticAppWrapper::Get(std::string name) {
    static CameraAppNonStaticAppWrapper instance(name);
    return instance;
}

CameraAppNonStaticAppWrapper::CameraAppNonStaticAppWrapper(std::string name) : StaticApp(name) {

}

void CameraAppNonStaticAppWrapper::Setup() {
    if (!mApp) {
        mApp = new CameraNonStaticApp(mName);
        mApp->Setup();
    }
}

void CameraAppNonStaticAppWrapper::Loop() {
    if (mApp) mApp->Loop();
}

void CameraAppNonStaticAppWrapper::UpdateButtons(int button) {
    if (mApp) mApp->UpdateButtons(button);
}

void CameraAppNonStaticAppWrapper::UpdateTouch(const TouchPoint* touches, int count) {
    if (mApp) mApp->UpdateTouch(touches, count);
}

void CameraAppNonStaticAppWrapper::Draw() {
    if (mApp) mApp->Draw();
}

const uint16_t* CameraAppNonStaticAppWrapper::getIcon() {
    return &Icons::Camera[0][0];
}

std::string CameraAppNonStaticAppWrapper::GetName() const {
    return mName;
}

void CameraAppNonStaticAppWrapper::SetName(const std::string& name) {
    mName = name;
}

void CameraAppNonStaticAppWrapper::CloseApp() {
    if (mApp) {
        delete mApp;
        mApp = nullptr;
    }
}

CameraAppNonStaticAppWrapper::~CameraAppNonStaticAppWrapper() {
    CloseApp();
}
