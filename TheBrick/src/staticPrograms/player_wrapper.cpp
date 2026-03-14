#include "staticPrograms/player_wrapper.h"

PlayerAppNonStaticAppWrapper &PlayerAppNonStaticAppWrapper::Get(std::string name) {
    static PlayerAppNonStaticAppWrapper instance(name);
    return instance;
}

PlayerAppNonStaticAppWrapper::PlayerAppNonStaticAppWrapper(std::string name) : StaticApp(name) {

}

void PlayerAppNonStaticAppWrapper::Setup() {
    if (!mApp) {
        mApp = new PlayerNonStaticApp(mName);
        mApp->Setup();
    }
}

void PlayerAppNonStaticAppWrapper::Loop() {
    if (mApp) mApp->Loop();
}

void PlayerAppNonStaticAppWrapper::UpdateButtons(int button) {
    if (mApp) mApp->UpdateButtons(button);
}

void PlayerAppNonStaticAppWrapper::UpdateTouch(const TouchPoint* touches, int count) {
    if (mApp) mApp->UpdateTouch(touches, count);
}

void PlayerAppNonStaticAppWrapper::Draw() {
    if (mApp) mApp->Draw();
}

const uint16_t* PlayerAppNonStaticAppWrapper::getIcon() {
    return &Icons::Player[0][0];
}

std::string PlayerAppNonStaticAppWrapper::GetName() const {
    return mName;
}

void PlayerAppNonStaticAppWrapper::SetName(const std::string& name) {
    mName = name;
}

void PlayerAppNonStaticAppWrapper::CloseApp() {
    if (mApp) {
        mApp->CloseApp();
        delete mApp;
        mApp = nullptr;
    }
}

PlayerAppNonStaticAppWrapper::~PlayerAppNonStaticAppWrapper() {
    CloseApp();
}
