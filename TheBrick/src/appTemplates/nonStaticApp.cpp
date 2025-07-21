#include "appTemplates/nonStaticApp.h"

NonStaticApp::NonStaticApp(std::string name) : mName(name) {}

std::string NonStaticApp::GetName() const {
    return mName;
}

void NonStaticApp::SetName(const std::string& name) {
    mName = name;
}
