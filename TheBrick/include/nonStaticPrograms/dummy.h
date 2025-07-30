#pragma once

#include "appTemplates/nonStaticApp.h"

class DummyNonStaticApp : public NonStaticApp {
public:
    DummyNonStaticApp(const std::string& name);

    void Loop() override;
    void UpdateButtons(int button) override;
    void UpdateTouch(const TouchPoint* touches, int count) override;
    void Setup() override;
    void Draw() override;
    const uint16_t* getIcon() override;

    static const uint16_t* StaticIcon();  // For wrapper

private:
    // Any dummy state if needed
};
    