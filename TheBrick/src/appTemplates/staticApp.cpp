#include "appTemplates/staticApp.h"

StaticApp::StaticApp(std::string name) : mName(name) {}

void StaticApp::Loop() {

}

void StaticApp::UpdateButtons(int button) {

}

void StaticApp::UpdateTouch(const TouchPoint* touches, int count) {

}

void StaticApp::Setup() {

}

void StaticApp::Draw() {

}

const unsigned char* StaticApp::getIcon() {
    return nullptr;
}

std::string StaticApp::GetName() const {
    return mName;
}

void StaticApp::SetName(const std::string& name) {
    mName = name;
}
