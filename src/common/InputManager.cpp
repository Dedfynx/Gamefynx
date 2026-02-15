#include "common/InputManager.h"
#include "common/EmulatorInterface.h"
#include "utils/Logger.h"

InputManager::InputManager() {
    buttonStates.fill(false);
}

void InputManager::processEvent(const SDL_Event& event) {
    if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP) {
        bool pressed = (event.type == SDL_EVENT_KEY_DOWN);

        // Essaye CHIP-8
        EmulatorButton chip8_btn = sdlKeyToChip8Button(event.key.key);
        if (chip8_btn != EmulatorButton::COUNT) {
            buttonStates[static_cast<size_t>(chip8_btn)] = pressed;
            return;
        }

        // Essaye Game Boy
        EmulatorButton gb_btn = sdlKeyToGameBoyButton(event.key.key);
        if (gb_btn != EmulatorButton::COUNT) {
            buttonStates[static_cast<size_t>(gb_btn)] = pressed;
            return;
        }
    }
}

void InputManager::updateEmulator(IEmulator* emulator, const std::string& coreName) {
    if (!emulator) return;

    if (coreName == "CHIP-8") {
        for (int i = 0; i < 16; ++i) {
            EmulatorButton btn = static_cast<EmulatorButton>(static_cast<int>(EmulatorButton::CHIP8_0) + i);
            emulator->setButton(i, buttonStates[static_cast<size_t>(btn)]);
        }
    } else if (coreName == "Game Boy") {

        emulator->setButton(0, buttonStates[static_cast<size_t>(EmulatorButton::GB_A)]);
        emulator->setButton(1, buttonStates[static_cast<size_t>(EmulatorButton::GB_B)]);
        emulator->setButton(2, buttonStates[static_cast<size_t>(EmulatorButton::GB_SELECT)]);
        emulator->setButton(3, buttonStates[static_cast<size_t>(EmulatorButton::GB_START)]);
        emulator->setButton(4, buttonStates[static_cast<size_t>(EmulatorButton::GB_RIGHT)]);
        emulator->setButton(5, buttonStates[static_cast<size_t>(EmulatorButton::GB_LEFT)]);
        emulator->setButton(6, buttonStates[static_cast<size_t>(EmulatorButton::GB_UP)]);
        emulator->setButton(7, buttonStates[static_cast<size_t>(EmulatorButton::GB_DOWN)]);
    }
}

bool InputManager::isKeyPressed(EmulatorButton button) const {
    return buttonStates[static_cast<size_t>(button)];
}

EmulatorButton InputManager::sdlKeyToChip8Button(SDL_Keycode key) {
    switch (key) {
        case SDLK_X: return EmulatorButton::CHIP8_0;
        case SDLK_1: return EmulatorButton::CHIP8_1;
        case SDLK_2: return EmulatorButton::CHIP8_2;
        case SDLK_3: return EmulatorButton::CHIP8_3;
        case SDLK_A: return EmulatorButton::CHIP8_4;
        case SDLK_Z: return EmulatorButton::CHIP8_5;
        case SDLK_E: return EmulatorButton::CHIP8_6;
        case SDLK_Q: return EmulatorButton::CHIP8_7;
        case SDLK_S: return EmulatorButton::CHIP8_8;
        case SDLK_D: return EmulatorButton::CHIP8_9;
        case SDLK_W: return EmulatorButton::CHIP8_A;
        case SDLK_C: return EmulatorButton::CHIP8_B;
        case SDLK_4: return EmulatorButton::CHIP8_C;
        case SDLK_R: return EmulatorButton::CHIP8_D;
        case SDLK_F: return EmulatorButton::CHIP8_E;
        case SDLK_V: return EmulatorButton::CHIP8_F;
        default: return EmulatorButton::COUNT;
    }
}

EmulatorButton InputManager::sdlKeyToGameBoyButton(SDL_Keycode key) {
    switch (key) {
        case SDLK_A: return EmulatorButton::GB_A;
        case SDLK_Z: return EmulatorButton::GB_B;
        case SDLK_R: return EmulatorButton::GB_START;
        case SDLK_T: return EmulatorButton::GB_SELECT;
        case SDLK_I: return EmulatorButton::GB_UP;
        case SDLK_K: return EmulatorButton::GB_DOWN;
        case SDLK_J: return EmulatorButton::GB_LEFT;
        case SDLK_L: return EmulatorButton::GB_RIGHT;
        default: return EmulatorButton::COUNT;
    }
}