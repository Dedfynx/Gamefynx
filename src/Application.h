#pragma once
#include <SDL3/SDL.h>
#include <memory>
#include <string>
#include <vector>

#include "common/EmulatorInterface.h"
#include "common/EmulatorCore.h"
#include "common/InputManager.h"
#include "ui/MainWindow.h"
#include "ui/ScreenRenderer.h"
#include "ui/Workspace.h"
#include "utils/audio.h"

class Application {
public:
    Application();
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    bool init();
    void run();
    void shutdown();

private:
    // Window
    SDL_Window* window = nullptr;
    SDL_GLContext gl_context = nullptr;
    bool running = false;

    // Timing
    float delta_time = 0.0f;
    uint64_t last_frame_time = 0;

    // Components
    Audio audio;
    InputManager input;
    MainWindow mainWindow;
    ScreenRenderer screenRenderer;
    Workspace workspace;

    // Emulation
    std::unique_ptr<IEmulator> emulator;
    EmulatorCore currentCore = EmulatorCore::None;
    std::vector<EmulatorCore> availableCores;
    bool rom_loaded = false;

    // Init
    bool initSDL();
    bool initOpenGL();
    bool initImGui();
    bool initEmulators();

    // Loop
    void processEvents();
    void update();
    void render();
    void handleUserActions();

    void updateDeltaTime();

    std::unique_ptr<IEmulator> createEmulator(EmulatorCore core);
    std::vector<EmulatorCore> getAvailableCores();
    std::vector<std::string> getFiltersForCore(EmulatorCore core);
};