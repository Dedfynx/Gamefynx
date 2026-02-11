#include <SDL3/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>
#include <SDL3/SDL_opengl.h>
#include <memory>

#include "imgui_internal.h"
#include "common/emulator_interface.h"
#include "ui/mainWindow.h"
#include "ui/screenRenderer.h"
#include "utils/logger.h"
#include "utils/file_utils.h"
#include "utils/Audio.h"

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
        return -1;
    }
}

int main(int argc, char *argv[])
{
    // ¯\_(ツ)_/¯
    (void)argv;
    (void)argc;

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        LOG_ERROR("SDL initialization failed: {}", SDL_GetError());
        return 1;
    }

    // Configuration OpenGL 3.3 Core
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // Création de la fenêtre
    SDL_Window *window = SDL_CreateWindow(
        "Gamefynx",
        1280, 768,
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
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    //
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 5.0f;
    style.FrameRounding = 3.0f;
    style.GrabRounding = 3.0f;
    style.ScrollbarRounding = 3.0f;
    style.WindowPadding = ImVec2(8, 8);
    style.FramePadding = ImVec2(5, 3);

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.15f, 0.15f, 0.17f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.15f, 0.17f, 1.00f);
    //
    ImGui_ImplSDL3_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    std::unique_ptr<IEmulator> emulator;

    Audio audio;
    if (!audio.init())
    {
        LOG_WARN("Audio initialization failed");
    }

#ifdef CORE_CHIP8_ENABLED
    emulator = std::make_unique<Chip8Emulator>();
    auto* chip8 = dynamic_cast<Chip8Emulator*>(emulator.get());
    if (chip8) {
        chip8->setAudio(&audio);
    }
    LOG_INFO("CHIP-8 emulator core loaded");
#endif
#ifdef CORE_GAMEBOY_ENABLED
    LOG_INFO("Game Boy emulator core loaded");
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

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
        window_flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        ImGui::Begin("DockSpace", nullptr, window_flags);
        ImGui::PopStyleVar(3);

        // DockSpace
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

        // ⚡ Menu bar (délégué à MainWindow)
        mainWindow.renderMenuBar();

        // Layout par défaut (première fois)
        static bool first_time = true;
        if (first_time) {
            first_time = false;

            ImGui::DockBuilderRemoveNode(dockspace_id);
            ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

            ImGuiID dock_main_id = dockspace_id;
            ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.25f, nullptr, &dock_main_id);
            ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.25f, nullptr, &dock_main_id);

            ImGui::DockBuilderDockWindow("CHIP-8 Screen", dock_main_id);
            ImGui::DockBuilderDockWindow("Controls", dock_id_left);
            ImGui::DockBuilderDockWindow("Stats", dock_id_right);

            ImGui::DockBuilderFinish(dockspace_id);
        }

        ImGui::End();
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

        if (mainWindow.shouldExit()) {
            running = false;
            mainWindow.clearFlags();
        }

        if (emulator && rom_loaded && !mainWindow.isPaused())
        {
            emulator->runFrame();
        }

        audio.update();

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