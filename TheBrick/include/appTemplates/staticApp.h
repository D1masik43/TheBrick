#pragma once
#include "appTemplates/appBase.h"

class StaticApp : public AppBase {
public:
    StaticApp(std::string name);

    std::string GetName() const override;
    void SetName(const std::string& name) override;

protected:
    std::string mName;
};
