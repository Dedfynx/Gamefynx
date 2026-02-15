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
    void renderMenuBar(EmulatorCore& currentCore, const std::vector<EmulatorCore>& availableCores);
    void renderStats(IEmulator* emulator);
    void renderControlPanel(IEmulator* emulator);
    void renderMemoryWatch(IEmulator* emulator);

    bool shouldLoadROM() const { return loadRomRequested; }
    bool shouldReset() const { return resetRequested; }
    bool shouldExit() const { return exitRequested; }
    bool shouldSelectCore() const { return selectCoreRequested; }
    bool isPaused() const { return paused; }

    EmulatorCore getRequestedCore() const { return requestedCore; }

    void clearFlags();
    
private:

    void displayMemoryGrid(IEmulator* emulator);

    
    bool showDemo = false;
    bool showAbout = false;
    bool showControls = true;
    bool showMemoryWatch = true;
    bool showStats = true;
    bool paused = false;
    
    bool loadRomRequested = false;
    bool resetRequested = false;
    bool exitRequested = false;
    bool selectCoreRequested = false;

    EmulatorCore requestedCore = EmulatorCore::None;
    
    float fps = 0.0f;
    int frames = 0;
};