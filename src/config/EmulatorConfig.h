#pragma once

namespace Config {
    // Window
    constexpr int DEFAULT_WINDOW_WIDTH = 1280;
    constexpr int DEFAULT_WINDOW_HEIGHT = 720;
    constexpr const char* WINDOW_TITLE = "Gamefynx";

    // OpenGL
    constexpr int GL_VERSION_MAJOR = 3;
    constexpr int GL_VERSION_MINOR = 3;

    // ImGui
    constexpr float DOCK_SPLIT_LEFT = 0.25f;
    constexpr float DOCK_SPLIT_RIGHT = 0.25f;

    // Emulation
    constexpr int CHIP8_CYCLES_PER_FRAME = 9;
    constexpr int GB_CYCLES_PER_FRAME = 70224;

    // Audio
    constexpr int AUDIO_SAMPLE_RATE = 44100;
    constexpr int AUDIO_BUFFER_SIZE = 1024;
}