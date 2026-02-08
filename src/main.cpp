#include <SDL3/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>
#include <SDL3/SDL_opengl.h>
#include <memory>

#include "common/emulator_interface.h"
#include "ui/main_window.h"
#include "ui/screen_renderer.h"
#include "utils/logger.h"
#include "utils/file_utils.h"

#ifdef CORE_CHIP8_ENABLED
#include "core/chip8/chip8.h"
#endif

int sdlKeyToChip8(SDL_Keycode key)
{
    switch (key)
    {
    case SDLK_1:
        return 0x1;
    case SDLK_2:
        return 0x2;
    case SDLK_3:
        return 0x3;
    case SDLK_4:
        return 0xC;

    case SDLK_A:
        return 0x4;
    case SDLK_Z:
        return 0x5;
    case SDLK_E:
        return 0x6;
    case SDLK_R:
        return 0xD;

    case SDLK_Q:
        return 0x7;
    case SDLK_S:
        return 0x8;
    case SDLK_D:
        return 0x9;
    case SDLK_F:
        return 0xE;

    case SDLK_W:
        return 0xA;
    case SDLK_X:
        return 0x0;
    case SDLK_C:
        return 0xB;
    case SDLK_V:
        return 0xF;

    default:
        return -1; // Touche non mappée
    }
}

int main(int argc, char *argv[])
{
    // ¯\_(ツ)_/¯
    (void)argv;
    (void)argc;

    // Initialisation SDL3
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        LOG_ERROR("SDL Error: {}", SDL_GetError());
        return 1;
    }

    // Configuration OpenGL 3.3 Core
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // Création de la fenêtre
    SDL_Window *window = SDL_CreateWindow(
        "Gamefynx - Multi-Emulator",
        1024, 768,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if (!window)
    {
        LOG_ERROR("Window creation failed: {}", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Contexte OpenGL
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if (!gl_context)
    {
        LOG_ERROR("OpenGL context creation failed: {}", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // VSync activé

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Navigation clavier

    // Style
    ImGui::StyleColorsDark();

    // Init backends ImGui
    ImGui_ImplSDL3_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // Crée l'émulateur
    std::unique_ptr<IEmulator> emulator;

#ifdef CORE_CHIP8_ENABLED
    emulator = std::make_unique<Chip8Emulator>();
    LOG_INFO("CHIP-8 emulator core loaded");
#else
    LOG_WARN("No emulator core enabled!");
#endif

    // UI Components
    MainWindow mainWindow;
    ScreenRenderer screenRenderer;

    // Main loop
    bool running = true;
    bool rom_loaded = false;

    while (running)
    {
        // Event handling
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);

            if (event.type == SDL_EVENT_QUIT)
            {
                running = false;
            }

            if (emulator && rom_loaded)
            {
                if (event.type == SDL_EVENT_KEY_DOWN)
                {
                    int chip8_key = sdlKeyToChip8(event.key.key);
                    if (chip8_key != -1)
                    {
                        emulator->setButton(chip8_key, true);
                        LOG_DEBUG("Key pressed: CHIP-8 key {:#x}", chip8_key);
                    }

                    if (event.key.key == SDLK_ESCAPE)
                    {
                        running = false;
                    }
                }

                if (event.type == SDL_EVENT_KEY_UP)
                {
                    int chip8_key = sdlKeyToChip8(event.key.key);
                    if (chip8_key != -1)
                    {
                        emulator->setButton(chip8_key, false);
                        LOG_DEBUG("Key released: CHIP-8 key {:#x}", chip8_key);
                    }
                }
            }
        }

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        // Render UI
        mainWindow.render(emulator.get());

        if (emulator && rom_loaded)
        {
            screenRenderer.render(
                emulator->getFramebuffer(),
                emulator->getScreenWidth(),
                emulator->getScreenHeight());
        }

        // Handle actions
        if (mainWindow.shouldLoadROM())
        {
            std::string rom_path = FileUtils::openFileDialog(
                "Select CHIP-8 ROM",
                {"CHIP-8 ROMs (*.ch8 *.c8)", "*.ch8 *.c8",
                 "All Files", "*"});

            if (!rom_path.empty())
            {
                if (emulator && emulator->loadROM(rom_path))
                {
                    rom_loaded = true;
                }
            }
            
            mainWindow.clearFlags();
        }

        if (mainWindow.shouldReset())
        {
            if (emulator)
            {
                emulator->reset();
                rom_loaded = false;
            }
            mainWindow.clearFlags();
        }

        if (emulator && rom_loaded && !mainWindow.isPaused())
        {
            emulator->runFrame();
        }

        // Rendering
        ImGui::Render();

        int display_w, display_h;
        SDL_GetWindowSizeInPixels(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);

        glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DestroyContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}