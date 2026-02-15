#pragma once
#include <SDL3/SDL.h>
#include <array>
#include <string>

enum class EmulatorButton {
    // CHIP-8 (16 boutons)
    CHIP8_0, CHIP8_1, CHIP8_2, CHIP8_3,
    CHIP8_4, CHIP8_5, CHIP8_6, CHIP8_7,
    CHIP8_8, CHIP8_9, CHIP8_A, CHIP8_B,
    CHIP8_C, CHIP8_D, CHIP8_E, CHIP8_F,

    // Game Boy
    GB_A, GB_B, GB_START, GB_SELECT,
    GB_UP, GB_DOWN, GB_LEFT, GB_RIGHT,

    COUNT
};

class IEmulator;

class InputManager {
public:
    InputManager();

    void processEvent(const SDL_Event& event);
    void updateEmulator(IEmulator* emulator, const std::string& core_name);

    bool isKeyPressed(EmulatorButton button) const;

private:
    std::array<bool, static_cast<size_t>(EmulatorButton::COUNT)> button_states{};

    EmulatorButton sdlKeyToChip8Button(SDL_Keycode key);
    EmulatorButton sdlKeyToGameBoyButton(SDL_Keycode key);
};