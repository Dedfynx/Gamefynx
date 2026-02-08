#include "ui/main_window.h"
#include <imgui.h>

#ifdef CORE_CHIP8_ENABLED
#include "core/chip8/chip8.h"
#endif

MainWindow::MainWindow() {
    // Initialisation si nécessaire
}

void MainWindow::render(IEmulator* emulator) {
    renderMenuBar();
    renderControlPanel(emulator);
    renderStats(emulator);
    
    // Fenêtre About (optionnel)
    if (show_about) {
        ImGui::Begin("About Gamefynx", &show_about);
        ImGui::Text("Gamefynx Multi-Emulator");
        ImGui::Text("Version 1.0.0");
        ImGui::Separator();
        ImGui::Text("Built with:");
        ImGui::BulletText("SDL3");
        ImGui::BulletText("Dear ImGui");
        ImGui::BulletText("OpenGL 3.3");
        ImGui::End();
    }
    
    // Demo ImGui (pour apprendre les widgets)
    if (show_demo) {
        ImGui::ShowDemoWindow(&show_demo);
    }
}

void MainWindow::renderMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Load ROM...", "Ctrl+O")) {
                load_rom_requested = true;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Esc")) {
                // Signal pour fermer l'app (géré dans main)
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Emulation")) {
            if (ImGui::MenuItem("Reset", "Ctrl+R")) {
                reset_requested = true;
            }
            if (ImGui::MenuItem("Pause/Resume", "Space")) {
                paused = !paused;
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Debug")) {
            ImGui::MenuItem("Show ImGui Demo", nullptr, &show_demo);
            // Futures options : CPU Viewer, Memory Viewer, etc.
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) {
                show_about = true;
            }
            ImGui::EndMenu();
        }
        
        ImGui::EndMainMenuBar();
    }
}

void MainWindow::renderControlPanel(IEmulator* emulator) {
    ImGui::Begin("Controls");
    
    if (emulator) {
        ImGui::Text("Emulator: %s", emulator->getArchName().c_str());
        ImGui::Separator();
        
        // Boutons de contrôle
        if (ImGui::Button(paused ? "Resume" : "Pause")) {
            paused = !paused;
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset")) {
            reset_requested = true;
        }
        
        ImGui::Separator();
        
        #ifdef CORE_CHIP8_ENABLED
        auto* chip8 = dynamic_cast<Chip8Emulator*>(emulator);
        if (chip8) {
            const auto& keys = chip8->getKeypad();
            
            const char* labels[16] = {
                "1", "2", "3", "C",
                "4", "5", "6", "D",
                "7", "8", "9", "E",
                "A", "0", "B", "F"
            };
            
            for (int row = 0; row < 4; ++row) {
                for (int col = 0; col < 4; ++col) {
                    int idx = row * 4 + col;
                    
                    if (col > 0) ImGui::SameLine();
                    
                    ImVec4 color = keys[idx] ? 
                        ImVec4(0.2f, 0.8f, 0.2f, 1.0f) :  //Vert
                        ImVec4(0.3f, 0.3f, 0.3f, 1.0f);   //Gris
                    
                    ImGui::PushStyleColor(ImGuiCol_Button, color);
                    ImGui::Button(labels[idx], ImVec2(40, 40));
                    ImGui::PopStyleColor();
                }
            }
        }
        #endif
        
        // Visualisation simple (peut être amélioré)
        ImGui::TextColored(
            ImVec4(0.5f, 1.0f, 0.5f, 1.0f), 
            "Press keys to test input"
        );
        
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "No emulator loaded");
        if (ImGui::Button("Load ROM")) {
            load_rom_requested = true;
        }
    }
    
    ImGui::End();
}
void MainWindow::renderStats(IEmulator* emulator) {
    ImGui::Begin("Stats");
    
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    
    if (emulator) {
        ImGui::Text("Screen: %dx%d", 
                    emulator->getScreenWidth(), 
                    emulator->getScreenHeight());
    }
    
    ImGui::End();
}

void MainWindow::clearFlags() {
    load_rom_requested = false;
    reset_requested = false;
}

