#include "staticPrograms/files_wrapper.h"

FilesAppNonStaticAppWrapper &FilesAppNonStaticAppWrapper::Get(std::string name) {
    static FilesAppNonStaticAppWrapper instance(name);
    return instance;
}

FilesAppNonStaticAppWrapper::FilesAppNonStaticAppWrapper(std::string name) : StaticApp(name) {

}

void FilesAppNonStaticAppWrapper::Setup() {
    if (!mApp) {
        mApp = new FilesNonStaticApp(mName);
        mApp->Setup();
    }
}

void FilesAppNonStaticAppWrapper::Loop() {
    if (mApp) mApp->Loop();
}

void FilesAppNonStaticAppWrapper::UpdateButtons(int button) {
    if (mApp) mApp->UpdateButtons(button);
}

void FilesAppNonStaticAppWrapper::UpdateTouch(const TouchPoint* touches, int count) {
    if (mApp) mApp->UpdateTouch(touches, count);
}

void FilesAppNonStaticAppWrapper::Draw() {
    if (mApp) mApp->Draw();
}

const uint16_t* FilesAppNonStaticAppWrapper::getIcon() {
    return &Icons::Files[0][0];
}

std::string FilesAppNonStaticAppWrapper::GetName() const {
    return mName;
}

void FilesAppNonStaticAppWrapper::SetName(const std::string& name) {
    mName = name;
}

void FilesAppNonStaticAppWrapper::CloseApp() {
    if (mApp) {
        mApp->CloseApp();
        delete mApp;
        mApp = nullptr;
    }
}

FilesAppNonStaticAppWrapper::~FilesAppNonStaticAppWrapper() {
    CloseApp();
}
