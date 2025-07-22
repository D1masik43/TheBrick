#pragma once
#include "appTemplates/appBase.h"

class NonStaticApp : public AppBase {
public:
    NonStaticApp(std::string name);

    void Loop() override;
    void UpdateButtons(int button) override;
    void UpdateTouch(const TouchPoint* touches, int count) override;
    void Setup() override;
    void Draw() override;
    const unsigned char* getIcon() override;
    
    std::string GetName() const override;
    void SetName(const std::string& name) override;

private:
    std::string mName;
};
