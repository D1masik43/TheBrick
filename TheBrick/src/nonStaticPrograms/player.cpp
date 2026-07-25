#include "nonStaticPrograms/player.h"
#include "staticPrograms/mainMenu.h"

PlayerNonStaticApp::PlayerNonStaticApp(const std::string& name) : NonStaticApp(name) {}

void PlayerNonStaticApp::Setup() {
    screenBuff = &SystemDrivers::Get().GetScreenBuff();
    screenBuff->fillScreen(TFT_BLACK);

    if (mcpAvailable && xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        auto &mcp = SystemDrivers::Get().GetMCP();
        mcp.pinMode(15, OUTPUT);
        mcp.digitalWrite(15, HIGH);
        xSemaphoreGive(i2cMutex);
    }

    Audio &audio = SystemDrivers::Get().GetAudio();
    audio.setPinout(42, 41, 2);
    audio.setVolume(volume);

    xTaskCreatePinnedToCore(
        [](void *pvParameters) {
            while (1) {
                SystemDrivers::Get().GetAudio().loop();
                vTaskDelay(1);
            }
        },
        "AudioTask", 4096, NULL, 2, &audioTaskHandle, 0
    );

    ScanSD();
    if(!playlist.empty()) {
        PlayTrack();
        TogglePause();
    }
}

void PlayerNonStaticApp::ScanSD() {
    playlist.clear();
    File root = SD_MMC.open("/");
    File file = root.openNextFile();
    while(file) {
        std::string fileName = file.name();
        if(fileName.find(".mp3") != std::string::npos || fileName.find(".MP3") != std::string::npos) {
            playlist.push_back("/" + fileName);
        }
        file = root.openNextFile();
    }
    Serial.printf("Found %d MP3 files\n", playlist.size());
}

void PlayerNonStaticApp::PlayTrack() {
    if(playlist.empty()) return;
    SystemDrivers::Get().GetAudio().connecttoFS(SD_MMC, playlist[currentTrackIndex].c_str());
}

void PlayerNonStaticApp::TogglePause() {
    SystemDrivers::Get().GetAudio().pauseResume();
}

void PlayerNonStaticApp::Loop() {
    //SystemDrivers::Get().GetAudio().loop();
    Draw();
}

void PlayerNonStaticApp::VolumeAdj(int delta) {
    int newVolume = volume + delta;

    if (newVolume >= 0 && newVolume <= 21) {
        volume = newVolume;
        SystemDrivers::Get().GetAudio().setVolume(volume);
    }
}

void PlayerNonStaticApp::UpdateButtons(int button) {
    if(button == BUTTON_BACK) SystemCommon::Get().SetNextApp(&MainMenu::Get());
    if(button == BUTTON_LEFT) { currentTrackIndex = (currentTrackIndex - 1 + playlist.size()) % playlist.size(); PlayTrack();TogglePause(); }
    if(button == BUTTON_RIGHT) { currentTrackIndex = (currentTrackIndex + 1) % playlist.size(); PlayTrack();TogglePause(); }
    if(button == BUTTON_IN) TogglePause();
    if(button == BUTTON_UP) VolumeAdj(1);
    if(button == BUTTON_DOWN) VolumeAdj(-1);
}

void PlayerNonStaticApp::UpdateTouch(const TouchPoint* touches, int count) {
    if (count <= 0) return;
    const TouchPoint& tp = touches[0];

    if (tp.type == TAP) {
        if (prevBtn.IsPressed(tp) != -1) { 
            currentTrackIndex = (currentTrackIndex - 1 + playlist.size()) % playlist.size(); 
            PlayTrack();
            TogglePause();
        }
        if (playBtn.IsPressed(tp) != -1) { TogglePause(); }
        if (nextBtn.IsPressed(tp) != -1) { 
            currentTrackIndex = (currentTrackIndex + 1) % playlist.size(); 
            PlayTrack();
            TogglePause();
        }
        if (volUpBtn.IsPressed(tp) != -1) { VolumeAdj(1); }
        if (volDownBtn.IsPressed(tp) != -1) { VolumeAdj(-1); }
    }

}

void PlayerNonStaticApp::Draw() {
    screenBuff->fillScreen(TFT_BLACK);
    screenBuff->setTextColor(TFT_WHITE);
    screenBuff->setTextSize(2);
    
    // Status Text
    screenBuff->drawString("Music Player", 60, 20);
    std::string volStr = "Vol: " + std::to_string(volume);
    screenBuff->drawString(volStr.c_str(), 60, 60);

    if(!playlist.empty()) {
        screenBuff->setTextSize(1);
        screenBuff->drawString("Now Playing:", 10, 80);
        screenBuff->drawString(playlist[currentTrackIndex].c_str(), 10, 100);
    } else {
        screenBuff->drawString("No MP3s found!", 40, 100);
    }

    // Draw Buttons
    prevBtn.Draw(*screenBuff);
    playBtn.Draw(*screenBuff);
    nextBtn.Draw(*screenBuff);
    volUpBtn.Draw(*screenBuff);
    volDownBtn.Draw(*screenBuff);
    
    // main loop handles pushSprite + status bar
}

void PlayerNonStaticApp::CloseApp() {
    SystemDrivers::Get().GetAudio().stopSong();
    if (audioTaskHandle) {
        vTaskDelete(audioTaskHandle);
        audioTaskHandle = nullptr;
    }
    if (mcpAvailable && xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        SystemDrivers::Get().GetMCP().digitalWrite(15, LOW);
        xSemaphoreGive(i2cMutex);
    }
    playlist.clear();
}

// Icon boilerplate
static const uint16_t player_icon[256] = {0}; 
const uint16_t* PlayerNonStaticApp::getIcon() { return player_icon; }
const uint16_t* PlayerNonStaticApp::StaticIcon() { return player_icon; }