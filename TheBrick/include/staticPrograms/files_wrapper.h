#pragma once
#include "appTemplates/staticApp.h"
#include "nonStaticPrograms/files.h"
#include "System/systemImages.h"


class FilesAppNonStaticAppWrapper : public StaticApp {
public:
    static FilesAppNonStaticAppWrapper &Get(std::string name = "Files");

    FilesAppNonStaticAppWrapper(const FilesAppNonStaticAppWrapper &) = delete;
    FilesAppNonStaticAppWrapper &operator=(const FilesAppNonStaticAppWrapper &) = delete;

    void Setup() override;
    void Loop() override;
    void UpdateButtons(int button) override;
    void UpdateTouch(const TouchPoint* touches, int count) override;
    void Draw() override;
    const uint16_t* getIcon() override;
    
    std::string GetName() const override;
    void SetName(const std::string& name) override;

    void CloseApp() override;

    ~FilesAppNonStaticAppWrapper();

private:
    FilesAppNonStaticAppWrapper(std::string name);
    FilesNonStaticApp* mApp = nullptr;
    std::string mName;
};
