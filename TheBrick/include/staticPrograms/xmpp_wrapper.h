#pragma once
#include "appTemplates/staticApp.h"
#include "nonStaticPrograms/xmpp.h"

class XmppAppNonStaticAppWrapper : public StaticApp {
public:
    static XmppAppNonStaticAppWrapper &Get(std::string name = "XMPP");
    XmppAppNonStaticAppWrapper(const XmppAppNonStaticAppWrapper &) = delete;
    XmppAppNonStaticAppWrapper &operator=(const XmppAppNonStaticAppWrapper &) = delete;

    void Setup() override;
    void Loop() override;
    void UpdateButtons(int button) override;
    void UpdateTouch(const TouchPoint* touches, int count) override;
    void Draw() override;
    const uint16_t* getIcon() override;
    std::string GetName() const override;
    void SetName(const std::string& name) override;
    void CloseApp() override;
    ~XmppAppNonStaticAppWrapper();

private:
    XmppAppNonStaticAppWrapper(std::string name);
    XmppNonStaticApp* mApp = nullptr;
    std::string mName;
};
