#include "appTemplates/staticApp.h"

StaticApp::StaticApp(std::string name) : mName(name) {}

std::string StaticApp::GetName() const {
    return mName;
}

void StaticApp::SetName(const std::string& name) {
    mName = name;
}
