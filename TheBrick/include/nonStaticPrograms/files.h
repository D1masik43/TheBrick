#pragma once
#include "appTemplates/nonStaticApp.h"
#include "System/systemGlobals.h"

class Mainmenu;
class Appmenu;

class FilesNonStaticApp : public NonStaticApp {
public:
    FilesNonStaticApp(const std::string& name);

    void Loop() override;
    void UpdateButtons(int button) override;
    void UpdateTouch(const TouchPoint* touches, int count) override;
    void Setup() override;
    void Draw() override;
    void CloseApp() override;

private:
    TFT_eSprite* screenBuff;
     std::vector<String> fileList;
    int selectedIndex = 0;
    bool displayingImage = false;
    int totalOffsetY = 0;
    int minOffsetY = 0;
    int maxOffsetY = 0;
    int itemHeight = 20;
    int lastTouchY = 0;
    bool isSliding = false;


    void listFiles();
    void drawFileList();
    void openImage(const String& filename);
};
