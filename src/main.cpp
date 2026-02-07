#include <SDL3/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>
#include <SDL3/SDL_opengl.h>
#include <iostream>
#include <memory>

#include "common/emulator_interface.h"
#include "ui/main_window.h"
#include "ui/screen_renderer.h"

#ifdef CORE_CHIP8_ENABLED
#include "core/chip8/chip8.h"
#endif

int main(int argc, char* argv[]) {
    // Initialisation SDL3
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Configuration OpenGL 3.3 Core
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // Création de la fenêtre
    SDL_Window* window = SDL_CreateWindow(
        "Gamefynx - Multi-Emulator",
        1024, 768,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );
    
    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // Contexte OpenGL
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if (!gl_context) {
        std::cerr << "OpenGL context creation failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1);  // VSync activé

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Navigation clavier
    
    // Style
    ImGui::StyleColorsDark();

    // Init backends ImGui
    ImGui_ImplSDL3_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // Crée l'émulateur
    std::unique_ptr<IEmulator> emulator;
    
#ifdef CORE_CHIP8_ENABLED
    emulator = std::make_unique<Chip8Emulator>();
    std::cout << "✓ CHIP-8 emulator loaded" << std::endl;
#else
    std::cout << "⚠ No emulator core enabled!" << std::endl;
#endif

    // UI Components
    MainWindow mainWindow;
    ScreenRenderer screenRenderer;

    // Main loop
    bool running = true;
    while (running) {
        // Event handling
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
            
            // Fermeture via touche Escape (optionnel)
            if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.key == SDLK_ESCAPE) {
                    running = false;
                }
            }
        }

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        // Render UI
        mainWindow.render(emulator.get());
        
        if (emulator) {
            screenRenderer.render(
                emulator->getFramebuffer(),
                emulator->getScreenWidth(),
                emulator->getScreenHeight()
            );
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

    std::cout << "Goodbye!" << std::endl;
    return 0;
}