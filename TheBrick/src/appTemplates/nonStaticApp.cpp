#include "appTemplates/nonStaticApp.h"

NonStaticApp::NonStaticApp(std::string name) : mName(name) {}

void NonStaticApp::Loop() {

}

void NonStaticApp::UpdateButtons(int button) {

}

void NonStaticApp::UpdateTouch(const TouchPoint* touches, int count) {

}

void NonStaticApp::Setup() {

}

void NonStaticApp::Draw() {

}

const uint16_t* NonStaticApp::getIcon() {
    return nullptr;
}

std::string NonStaticApp::GetName() const {
    return mName;
}

void NonStaticApp::SetName(const std::string& name) {
    mName = name;
}
