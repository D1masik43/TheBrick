#pragma once
#include "appTemplates/staticApp.h"
#include "nonStaticPrograms/player.h"
#include "System/systemImages.h"


class PlayerAppNonStaticAppWrapper : public StaticApp {
public:
    static PlayerAppNonStaticAppWrapper &Get(std::string name = "Audio");

    PlayerAppNonStaticAppWrapper(const PlayerAppNonStaticAppWrapper &) = delete;
    PlayerAppNonStaticAppWrapper &operator=(const PlayerAppNonStaticAppWrapper &) = delete;

    void Setup() override;
    void Loop() override;
    void UpdateButtons(int button) override;
    void UpdateTouch(const TouchPoint* touches, int count) override;
    void Draw() override;
    const uint16_t* getIcon() override;
    
    std::string GetName() const override;
    void SetName(const std::string& name) override;

    void CloseApp() override;

    ~PlayerAppNonStaticAppWrapper();

private:
    PlayerAppNonStaticAppWrapper(std::string name);
    PlayerNonStaticApp* mApp = nullptr;
    std::string mName;
};
