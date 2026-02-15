#include "Application.h"
#include "config/EmulatorConfig.h"
#include "config/ImguiConfig.h"
#include "utils/Logger.h"
#include "utils/FileUtils.h"
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>
#include <SDL3/SDL_opengl.h>

#ifdef CORE_CHIP8_ENABLED
#include "core/chip8/Chip8.h"
#endif

#ifdef CORE_GAMEBOY_ENABLED
#include "core/gameboy/Gameboy.h"
#endif

Application::Application() {}
Application::~Application() { shutdown(); }

bool Application::init() {
    LOG_INFO("Initializing Gamefynx...");

    if (!initSDL()) return false;
    if (!initOpenGL()) return false;
    if (!initImGui()) return false;
    if (!initEmulators()) return false;

    if (!audio.init()) {
        LOG_WARN("Audio init failed, continuing without sound");
    }

    running = true;
    last_frame_time = SDL_GetPerformanceCounter();

    LOG_INFO("Application ready");
    return true;
}

bool Application::initSDL() {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        LOG_ERROR("SDL init failed: {}", SDL_GetError());
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, Config::GL_VERSION_MAJOR);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, Config::GL_VERSION_MINOR);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    window = SDL_CreateWindow(
        Config::WINDOW_TITLE,
        Config::DEFAULT_WINDOW_WIDTH,
        Config::DEFAULT_WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );

    if (!window) {
        LOG_ERROR("Window creation failed: {}", SDL_GetError());
        return false;
    }

    LOG_INFO("SDL initialized");
    return true;
}

bool Application::initOpenGL() {
    gl_context = SDL_GL_CreateContext(window);
    if (!gl_context) {
        LOG_ERROR("OpenGL context failed: {}", SDL_GetError());
        return false;
    }

    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1);

    LOG_INFO("OpenGL context created");
    return true;
}

bool Application::initImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGuiConfig::setupStyle();

    ImGui_ImplSDL3_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    LOG_INFO("ImGui initialized");
    return true;
}

bool Application::initEmulators() {
    availableCores = getAvailableCores();
    if (availableCores.empty()) {
        LOG_ERROR("No emulator cores available!");
        return false;
    }

    currentCore = availableCores[1];
    emulator = createEmulator(currentCore);

    return true;
}

void Application::run() {
    while (running) {
        updateDeltaTime();
        processEvents();
        update();
        render();
    }
}

void Application::processEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL3_ProcessEvent(&event);

        if (event.type == SDL_EVENT_QUIT) {
            running = false;
        }

        if (!ImGui::GetIO().WantCaptureKeyboard) {
            input.processEvent(event);
        }
    }
}

void Application::update() {
    audio.update();
    input.updateEmulator(emulator.get(), emulator ? emulator->getArchName() : "");

    if (emulator && rom_loaded && !mainWindow.isPaused()) {
        emulator->runFrame();
    }
}

void Application::render() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    // Docking
    workspace.begin();
    mainWindow.renderMenuBar(currentCore, availableCores);
    workspace.end();

    // UI
    mainWindow.render(emulator.get());

    if (emulator && rom_loaded) {
        screenRenderer.render(
            emulator->getFramebuffer(),
            emulator->getScreenWidth(),
            emulator->getScreenHeight()
        );
    }

    if (emulator) {
        mainWindow.renderMemoryWatch(emulator.get());
    }

    handleUserActions();

    // OpenGL
    ImGui::Render();

    int w, h;
    SDL_GetWindowSizeInPixels(window, &w, &h);
    glViewport(0, 0, w, h);
    glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(window);
}

void Application::handleUserActions() {
    if (mainWindow.shouldSelectCore()) {
        currentCore = mainWindow.getRequestedCore();
        emulator = createEmulator(currentCore);
        rom_loaded = false;
        mainWindow.clearFlags();
    }

    if (mainWindow.shouldLoadROM()) {
        auto filters = getFiltersForCore(currentCore);
        std::string romPath = FileUtils::openFileDialog("Select ROM", filters);

        if (!romPath.empty() && emulator) {
            if (emulator->loadROM(romPath)) {
                LOG_INFO("ROM loaded: {}", romPath);
                rom_loaded = true;
            } else {
                LOG_ERROR("Failed to load ROM: {}", romPath);
            }
        }
        mainWindow.clearFlags();
    }

    if (mainWindow.shouldReset()) {
        if (emulator && rom_loaded) emulator->reset();
        mainWindow.clearFlags();
    }

    if (mainWindow.shouldExit()) {
        running = false;
        mainWindow.clearFlags();
    }
}

void Application::updateDeltaTime() {
    uint64_t current = SDL_GetPerformanceCounter();
    delta_time = static_cast<float>(current - last_frame_time) / SDL_GetPerformanceFrequency();
    last_frame_time = current;
}

void Application::shutdown() {
    emulator.reset();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    if (gl_context) {
        SDL_GL_DestroyContext(gl_context);
        gl_context = nullptr;
    }

    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    SDL_Quit();
    LOG_INFO("Application shutdown complete");
}

std::unique_ptr<IEmulator> Application::createEmulator(EmulatorCore core) {
    std::unique_ptr<IEmulator> emu;

    switch (core) {
#ifdef CORE_CHIP8_ENABLED
    case EmulatorCore::CHIP8:
        emu = std::make_unique<Chip8>();
        if (auto* chip8 = dynamic_cast<Chip8*>(emu.get())) {
            chip8->setAudio(&audio);
        }
        LOG_INFO("CHIP-8 core loaded");
        break;
#endif
#ifdef CORE_GAMEBOY_ENABLED
    case EmulatorCore::GameBoy:
        emu = std::make_unique<Gameboy>();
        LOG_INFO("Game Boy core loaded");
        break;
#endif
    default:
        LOG_WARN("Unknown core");
        break;
    }

    return emu;
}

std::vector<EmulatorCore> Application::getAvailableCores() {
    std::vector<EmulatorCore> cores;
#ifdef CORE_CHIP8_ENABLED
    cores.push_back(EmulatorCore::CHIP8);
#endif
#ifdef CORE_GAMEBOY_ENABLED
    cores.push_back(EmulatorCore::GameBoy);
#endif
    return cores;
}

std::vector<std::string> Application::getFiltersForCore(EmulatorCore core) {
    switch (core) {
    case EmulatorCore::CHIP8:
        return {"CHIP-8 ROMs (*.ch8 *.c8)", "*.ch8 *.c8", "All Files", "*"};
    case EmulatorCore::GameBoy:
        return {"Game Boy ROMs (*.gb *.gbc)", "*.gb *.gbc", "All Files", "*"};
    default:
        return {"All Files", "*"};
    }
}