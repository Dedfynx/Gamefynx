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
    if (showControls) renderControlPanel(emulator);
    if (showMemoryWatch) renderMemoryWatch(emulator);
    if (showStats) renderStats(emulator);

    if (showAbout)
    {
        ImGui::Begin("About Gamefynx", &showAbout);
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
    if (showDemo)
    {
        ImGui::ShowDemoWindow(&showDemo);
    }
}

void MainWindow::renderMenuBar(EmulatorCore& currentCore, const std::vector<EmulatorCore>& availableCores)
{

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::BeginMenu("Select Core"))
            {
                for (auto core : availableCores) {
                    bool is_selected = (core == currentCore);
                    if (ImGui::MenuItem(getCoreNameStr(core), nullptr, is_selected)) {
                        if (core != currentCore) {
                            requestedCore = core;
                            selectCoreRequested = true;
                        }
                    }
                }
                ImGui::EndMenu();
            }
            if (ImGui::MenuItem("Load ROM...", "Ctrl+O"))
            {
                loadRomRequested = true;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Esc"))
            {
                exitRequested = true;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Emulation"))
        {
            if (ImGui::MenuItem("Reset", "Ctrl+R"))
            {
                resetRequested = true;
            }
            if (ImGui::MenuItem(paused ? "Resume" : "Pause", "Space"))
            {
                paused = !paused;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Debug"))
        {
            ImGui::MenuItem("Show ImGui Demo", nullptr, &showDemo);
            ImGui::MenuItem("Show Control", nullptr, &showControls);
            ImGui::MenuItem("Show Memory Watch", nullptr, &showMemoryWatch);
            ImGui::MenuItem("Show Stats", nullptr, &showStats);

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help"))
        {
            ImGui::MenuItem("About", nullptr, &showAbout);
            ImGui::EndMenu();
        }
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "Core: %s", getCoreNameStr(currentCore));

        ImGui::EndMainMenuBar();
    }
}

void MainWindow::renderControlPanel(IEmulator *emulator)
{
    ImGui::Begin("Controls", &showControls);

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
            resetRequested = true;
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
            loadRomRequested = true;
        }
    }

    ImGui::End();
}

void MainWindow::renderMemoryWatch(IEmulator* emulator) {
    if (!emulator) return;

    ImGui::Begin("Universal Debugger", &showMemoryWatch);
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
    ImGui::Begin("Stats", &showStats);

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
    loadRomRequested = false;
    resetRequested = false;
    exitRequested = false;
    selectCoreRequested = false;
}
