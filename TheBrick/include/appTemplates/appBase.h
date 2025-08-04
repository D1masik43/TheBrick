#pragma once
#include <string>
#include "System/systemStructs.h"

class AppBase {
public:
    virtual ~AppBase() {}

    virtual void Loop() = 0;
    virtual void UpdateButtons(int button) = 0;
    virtual void UpdateTouch(const TouchPoint* touches, int count) = 0;
    virtual void Setup() = 0;
    virtual void Draw() = 0;
    virtual const uint16_t* getIcon() { return nullptr; }

    virtual std::string GetName() const = 0;
    virtual void SetName(const std::string& name) = 0;

    virtual void CloseApp();
};
