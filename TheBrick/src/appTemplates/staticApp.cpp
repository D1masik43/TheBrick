#include "appTemplates/staticApp.h"

StaticApp::StaticApp(std::string name) : mName(name)
{
}

std::string StaticApp::GetName()
{
    return mName;
}
void StaticApp::SetName(std::string name)
{
    mName = name;
}

const unsigned char *StaticApp::getIcon()
{
    return nullptr;
}
