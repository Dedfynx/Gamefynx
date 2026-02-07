#include "ui/main_window.h"
#include <imgui.h>

void MainWindow::render(IEmulator* emulator) {
    ImGui::Begin("Multi-Emulator");
    
    if (emulator) {
        ImGui::Text("Current: %s", emulator->getArchName().c_str());
    } else {
        ImGui::Text("No emulator loaded");
    }
    
    ImGui::Separator();
    
    if (ImGui::Button("Load ROM")) {
        // TODO: file dialog
    }
    
    ImGui::Checkbox("Show ImGui Demo", &show_demo);
    
    ImGui::End();
    
    if (show_demo) {
        ImGui::ShowDemoWindow(&show_demo);
    }
}