#pragma once
#include "common/emulator_interface.h"
#include <string>

class MainWindow {
public:
    MainWindow();
    
    // Appelé chaque frame pour afficher l'UI
    void render(IEmulator* emulator);
    
    // Getters pour savoir ce que l'utilisateur a fait
    bool shouldLoadROM() const { return load_rom_requested; }
    bool shouldReset() const { return reset_requested; }
    bool isPaused() const { return paused; }
    
    void clearFlags();  // Reset les flags après traitement
    
private:
    void renderMenuBar();
    void renderControlPanel(IEmulator* emulator);
    void renderStats(IEmulator* emulator);
    
    // État UI
    bool show_demo = false;
    bool show_about = false;
    bool paused = false;
    
    // Flags d'action
    bool load_rom_requested = false;
    bool reset_requested = false;
    
    // Stats
    float fps = 0.0f;
    int frames = 0;
};