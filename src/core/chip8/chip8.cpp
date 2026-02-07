#include "core/chip8/chip8.h"
#include <fstream>
#include <cstring>

Chip8Emulator::Chip8Emulator() {
    reset();
}

bool Chip8Emulator::loadROM(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return false;
    }
    
    size_t size = file.tellg();
    file.seekg(0);
    
    if (size > 4096 - 0x200) {
        return false; 
    }
    
    file.read(reinterpret_cast<char*>(&memory[0x200]), size);
    return true;
}

void Chip8Emulator::reset() {
    memory.fill(0);
    V.fill(0);
    display.fill(0);
    keypad.fill(0);
    stack.fill(0);
    
    I = 0;
    pc = 0x200;
    sp = 0;
    delay_timer = 0;
    sound_timer = 0;
    
    // TODO: Load font sprites (0x000-0x1FF)
}

void Chip8Emulator::step() {
    // TODO: fetch-decode-execute
}

void Chip8Emulator::runFrame() {
    // TODO: run ~60 instructions (CHIP-8 ~540Hz, 60fps = ~9 cycles/frame)
    for (int i = 0; i < 9; ++i) {
        step();
    }
}

const uint8_t* Chip8Emulator::getFramebuffer() const {
    return display.data();
}

void Chip8Emulator::setButton(int button, bool pressed) {
    if (button >= 0 && button < 16) {
        keypad[button] = pressed ? 1 : 0;
    }
}

void Chip8Emulator::executeOpcode(uint16_t opcode) {
    // TODO: implémentation opcodes
}