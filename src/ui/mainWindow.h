#pragma once
#include "common/emulator_interface.h"
#include <string>

class MainWindow {
public:
    MainWindow() = default;

    void render(IEmulator* emulator);
    void renderMenuBar();

    bool shouldLoadROM() const { return load_rom_requested; }
    bool shouldReset() const { return reset_requested; }
    bool shouldExit() const { return exit_requested; }
    bool isPaused() const { return paused; }
    
    void clearFlags();
    
private:
    void renderControlPanel(IEmulator* emulator);
    void renderMemoryWatch(IEmulator* emulator);
    void displayMemoryGrid(IEmulator* emu);
    void renderStats(IEmulator* emulator);
    
    bool show_demo = false;
    bool show_about = false;
    bool show_controls = true;
    bool show_memory_watch = true;
    bool show_stats = true;
    bool paused = false;
    
    bool load_rom_requested = false;
    bool reset_requested = false;
    bool exit_requested = false;
    
    float fps = 0.0f;
    int frames = 0;
};