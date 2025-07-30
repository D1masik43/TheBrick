#include "nonStaticPrograms/dummy.h"

// Optionally define a static icon
static const uint16_t dummy_icon[16 * 16 / 8] = { 0 };  // blank 16x16 monochrome

DummyNonStaticApp::DummyNonStaticApp(const std::string& name)
    : NonStaticApp(name) {}

void DummyNonStaticApp::Loop() {
    // Nothing
}

void DummyNonStaticApp::UpdateButtons(int button) {
    // Nothing
}

void DummyNonStaticApp::UpdateTouch(const TouchPoint* touches, int count) {
    // Nothing
}

void DummyNonStaticApp::Setup() {
    // Nothing
}

void DummyNonStaticApp::Draw() {
    // Nothing
}

const uint16_t* DummyNonStaticApp::getIcon() {
    return dummy_icon;
}

const uint16_t* DummyNonStaticApp::StaticIcon() {
    return dummy_icon;
}
