#pragma once
#include "appTemplates/staticApp.h"
#include "nonStaticPrograms/camera.h"
#include "System/systemImages.h"


class CameraAppNonStaticAppWrapper : public StaticApp {
public:
    static CameraAppNonStaticAppWrapper &Get(std::string name = "Camera");

    CameraAppNonStaticAppWrapper(const CameraAppNonStaticAppWrapper &) = delete;
    CameraAppNonStaticAppWrapper &operator=(const CameraAppNonStaticAppWrapper &) = delete;

    void Setup() override;
    void Loop() override;
    void UpdateButtons(int button) override;
    void UpdateTouch(const TouchPoint* touches, int count) override;
    void Draw() override;
    const uint16_t* getIcon() override;
    
    std::string GetName() const override;
    void SetName(const std::string& name) override;

    void CloseApp() override;

    ~CameraAppNonStaticAppWrapper();

private:
    CameraAppNonStaticAppWrapper(std::string name);
    CameraNonStaticApp* mApp = nullptr;
    std::string mName;
};
