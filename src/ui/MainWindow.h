#pragma once
#include "common/EmulatorInterface.h"
#include "common/EmulatorCore.h"
#include <string>
#include <vector>


const char* getCoreNameStr(EmulatorCore core);

class MainWindow {
public:
    MainWindow() = default;

    void render(IEmulator* emulator);
    void renderMenuBar(EmulatorCore& current_core, const std::vector<EmulatorCore>& available_cores);
    void renderStats(IEmulator* emulator);
    void renderControlPanel(IEmulator* emulator);
    void renderMemoryWatch(IEmulator* emulator);

    bool shouldLoadROM() const { return load_rom_requested; }
    bool shouldReset() const { return reset_requested; }
    bool shouldExit() const { return exit_requested; }
    bool shouldSelectCore() const { return select_core_requested; }
    bool isPaused() const { return paused; }

    EmulatorCore getRequestedCore() const { return requested_core; }

    void clearFlags();
    
private:

    void displayMemoryGrid(IEmulator* emulator);

    
    bool show_demo = false;
    bool show_about = false;
    bool show_controls = true;
    bool show_memory_watch = true;
    bool show_stats = true;
    bool paused = false;
    
    bool load_rom_requested = false;
    bool reset_requested = false;
    bool exit_requested = false;
    bool select_core_requested = false;

    EmulatorCore requested_core = EmulatorCore::None;
    
    float fps = 0.0f;
    int frames = 0;
};