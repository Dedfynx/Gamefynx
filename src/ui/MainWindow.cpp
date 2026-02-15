#include "ui/MainWindow.h"
#include <imgui.h>

#ifdef CORE_CHIP8_ENABLED
#include "core/chip8/Chip8.h"
#endif
#ifdef CORE_GAMEBOY_ENABLED
#include "core/gameboy/Gameboy.h"
#endif

const char* getCoreNameStr(EmulatorCore core) {
    switch (core) {
    case EmulatorCore::CHIP8: return "CHIP-8";
    case EmulatorCore::GameBoy: return "Game Boy";
    default: return "None";
    }
}

void MainWindow::render(IEmulator *emulator)
{
    //renderMenuBar();
    if (show_controls) renderControlPanel(emulator);
    if (show_memory_watch) renderMemoryWatch(emulator);
    if (show_stats) renderStats(emulator);

    if (show_about)
    {
        ImGui::Begin("About Gamefynx", &show_about);
        ImGui::Text("Gamefynx");
        ImGui::Text("Version 1.0.0");
        ImGui::Separator();
        ImGui::Text("Built with:");
        ImGui::BulletText("SDL3");
        ImGui::BulletText("Dear ImGui");
        ImGui::BulletText("OpenGL 3.3");
        ImGui::End();
    }

    // Demo ImGui (pour apprendre les widgets)
    if (show_demo)
    {
        ImGui::ShowDemoWindow(&show_demo);
    }
}

void MainWindow::renderMenuBar(EmulatorCore& current_core, const std::vector<EmulatorCore>& available_cores)
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::BeginMenu("Select Core"))
            {
                for (auto core : available_cores) {
                    bool is_selected = (core == current_core);
                    if (ImGui::MenuItem(getCoreNameStr(core), nullptr, is_selected)) {
                        if (core != current_core) {
                            requested_core = core;
                            select_core_requested = true;
                        }
                    }
                }
                ImGui::EndMenu();
            }
            if (ImGui::MenuItem("Load ROM...", "Ctrl+O"))
            {
                load_rom_requested = true;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Esc"))
            {
                exit_requested = true;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Emulation"))
        {
            if (ImGui::MenuItem("Reset", "Ctrl+R"))
            {
                reset_requested = true;
            }
            if (ImGui::MenuItem(paused ? "Resume" : "Pause", "Space"))
            {
                paused = !paused;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Debug"))
        {
            ImGui::MenuItem("Show ImGui Demo", nullptr, &show_demo);
            ImGui::MenuItem("Show Control", nullptr, &show_controls);
            ImGui::MenuItem("Show Memory Watch", nullptr, &show_memory_watch);
            ImGui::MenuItem("Show Stats", nullptr, &show_stats);

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help"))
        {
            ImGui::MenuItem("About", nullptr, &show_about);
            ImGui::EndMenu();
        }
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "Core: %s", getCoreNameStr(current_core));

        ImGui::EndMainMenuBar();
    }
}

void MainWindow::renderControlPanel(IEmulator *emulator)
{
    ImGui::Begin("Controls", &show_controls);

    if (emulator)
    {
        ImGui::Text("Emulator: %s", emulator->getArchName().c_str());
        ImGui::Separator();

        // Boutons de contrôle
        if (ImGui::Button(paused ? "Resume" : "Pause"))
        {
            paused = !paused;
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset"))
        {
            reset_requested = true;
        }

        ImGui::Separator();

#ifdef CORE_CHIP8_ENABLED
        auto *chip8 = dynamic_cast<Chip8 *>(emulator);
        if (chip8)
        {
            const auto &keys = chip8->getKeypad();

            const char *labels[16] = {
                "1", "2", "3", "C",
                "4", "5", "6", "D",
                "7", "8", "9", "E",
                "A", "0", "B", "F"};

            const int hexValues[16] = {
                0x1, 0x2, 0x3, 0xC,
                0x4, 0x5, 0x6, 0xD,
                0x7, 0x8, 0x9, 0xE,
                0xA, 0x0, 0xB, 0xF};

            for (int row = 0; row < 4; ++row)
            {
                for (int col = 0; col < 4; ++col)
                {
                    int idx = row * 4 + col;
                    int hexKey = hexValues[idx];

                    if (col > 0)
                        ImGui::SameLine();

                    ImVec4 color = keys[hexKey] ? ImVec4(0.2f, 0.8f, 0.2f, 1.0f) : // Vert
                                       ImVec4(0.3f, 0.3f, 0.3f, 1.0f);             // Gris

                    ImGui::PushStyleColor(ImGuiCol_Button, color);
                    ImGui::Button(labels[idx], ImVec2(40, 40));
                    ImGui::PopStyleColor();
                }
            }
        }
#endif
    }
    else
    {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "No emulator loaded");
        if (ImGui::Button("Load ROM"))
        {
            load_rom_requested = true;
        }
    }

    ImGui::End();
}

void MainWindow::renderMemoryWatch(IEmulator* emulator) {
    if (!emulator) return;

    ImGui::Begin("Universal Debugger", &show_memory_watch);
    ImGui::Text("System: %s", emulator->getArchName().c_str());
    ImGui::Text("PC: 0x%03X", emulator->getPC());
    ImGui::Separator();

#ifdef CORE_CHIP8_ENABLED
    if (auto* chip8 = dynamic_cast<Chip8*>(emulator)) {
        if (ImGui::CollapsingHeader("CHIP-8 Registers", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Columns(4, nullptr, false);
            for (int i = 0; i < 16; i++) {
                ImGui::Text("V%X: %02X", i, chip8->getV(i));
                ImGui::NextColumn();
            }
            ImGui::Columns(1);
            ImGui::Text("I: 0x%03X", chip8->getI());
        }
    }
#endif
#ifdef CORE_GAMEBOY_ENABLED
    if (auto* gb = dynamic_cast<Gameboy*>(emulator)) {
        if (ImGui::CollapsingHeader("Game Boy Registers", ImGuiTreeNodeFlags_DefaultOpen)) {
            const auto& cpu = gb->getCPU();

            ImGui::Columns(2, nullptr, false);

            // Registres 16-bit
            ImGui::Text("AF: 0x%04X", cpu.af);
            ImGui::NextColumn();
            ImGui::Text("BC: 0x%04X", cpu.bc);
            ImGui::NextColumn();

            ImGui::Text("DE: 0x%04X", cpu.de);
            ImGui::NextColumn();
            ImGui::Text("HL: 0x%04X", cpu.hl);
            ImGui::NextColumn();

            ImGui::Text("SP: 0x%04X", cpu.sp);
            ImGui::NextColumn();
            ImGui::Text("PC: 0x%04X", cpu.pc);
            ImGui::NextColumn();

            ImGui::Columns(1);
            ImGui::Separator();

            // Flags
            uint8_t flags = cpu.f;
            ImGui::Text("Flags: %c%c%c%c",
                flags & 0x80 ? 'Z' : '-',
                flags & 0x40 ? 'N' : '-',
                flags & 0x20 ? 'H' : '-',
                flags & 0x10 ? 'C' : '-'
            );

            ImGui::Separator();
            ImGui::Text("Cycles: %d", cpu.getCycles());
            ImGui::Text("Halted: %s", cpu.isHalted() ? "Yes" : "No");
        }
    }
#endif

    if (ImGui::CollapsingHeader("Memory Watch", ImGuiTreeNodeFlags_DefaultOpen)) {
        displayMemoryGrid(emulator);
    }

    ImGui::End();
}

void MainWindow::displayMemoryGrid(IEmulator* emulator) {
    static bool autoScroll = true;
    ImGui::Checkbox("Follow PC", &autoScroll);

    const uint8_t* mem = emulator->getMemoryPtr();
    size_t memSize = emulator->getMemorySize();
    uint16_t pc = emulator->getPC();

    if (ImGui::BeginTable("MemTable", 17, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(0, 300))) {
        ImGui::TableSetupColumn("Addr", ImGuiTableColumnFlags_WidthFixed, 50.0f);
        for (int i = 0; i < 16; i++) ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthFixed, 25.0f);
        ImGui::TableHeadersRow();

        // On itère par blocs de 16 octets
        for (size_t addr = 0; addr < memSize; addr += 16) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextDisabled("%03X", addr);

            for (auto col = 0; col < 16; col++) {
                size_t currentAddr = addr + col;
                if (currentAddr >= memSize) break;

                ImGui::TableSetColumnIndex(col + 1);

                if (currentAddr == pc) {
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImVec4(0.8f, 0.2f, 0.2f, 0.6f)));
                    if (autoScroll) ImGui::SetScrollHereY();
                }

                ImGui::Text("%02X", mem[currentAddr]);
            }
        }
        ImGui::EndTable();
    }
}

void MainWindow::renderStats(IEmulator *emulator)
{
    ImGui::Begin("Stats", &show_stats);

    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

    if (emulator)
    {
        ImGui::Text("Screen: %dx%d",
                    emulator->getScreenWidth(),
                    emulator->getScreenHeight());
    }

    ImGui::End();
}

void MainWindow::clearFlags()
{
    load_rom_requested = false;
    reset_requested = false;
    exit_requested = false;
    select_core_requested = false;
}
