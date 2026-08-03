#include "staticPrograms/xmpp_wrapper.h"

XmppAppNonStaticAppWrapper &XmppAppNonStaticAppWrapper::Get(std::string name) {
    static XmppAppNonStaticAppWrapper instance(name);
    return instance;
}

XmppAppNonStaticAppWrapper::XmppAppNonStaticAppWrapper(std::string name) : StaticApp(name) {}

void XmppAppNonStaticAppWrapper::Setup() {
    if (!mApp) {
        mApp = new XmppNonStaticApp(mName);
        mApp->Setup();
    }
}

void XmppAppNonStaticAppWrapper::Loop() { if (mApp) mApp->Loop(); }
void XmppAppNonStaticAppWrapper::UpdateButtons(int button) { if (mApp) mApp->UpdateButtons(button); }
void XmppAppNonStaticAppWrapper::UpdateTouch(const TouchPoint* touches, int count) { if (mApp) mApp->UpdateTouch(touches, count); }
void XmppAppNonStaticAppWrapper::Draw() { if (mApp) mApp->Draw(); }
const uint16_t* XmppAppNonStaticAppWrapper::getIcon() { return &Icons::XMPP[0][0]; }
std::string XmppAppNonStaticAppWrapper::GetName() const { return mName; }
void XmppAppNonStaticAppWrapper::SetName(const std::string& name) { mName = name; }

void XmppAppNonStaticAppWrapper::CloseApp() {
    if (mApp) {
        mApp->CloseApp();
        delete mApp;
        mApp = nullptr;
    }
}

XmppAppNonStaticAppWrapper::~XmppAppNonStaticAppWrapper() { CloseApp(); }
