//
// Created by Dedfynx on 11/02/2026.
//

#include "Gameboy.h"
#include "utils/logger.h"

Gameboy::Gameboy() : cpu(memory) {
    framebuffer.fill(0xFF);  // Blanc par défaut
    LOG_DEBUG("Game Boy emulator created");
}

bool Gameboy::loadROM(const std::string& path) {
    LOG_INFO("Loading Game Boy ROM: {}", path);

    //todo : Boot Rom
    /*
    if (!memory.loadBootROM("roms/gameboy/dmg_boot.bin")) {
        LOG_WARN("Boot ROM not found, skipping");
    }
    */

    if (!memory.loadROM(path)) {
        return false;
    }

    reset();
    rom_loaded = true;

    return true;
}

void Gameboy::reset() {
    memory.reset();
    cpu.reset();
    framebuffer.fill(0xFF);
    LOG_DEBUG("Game Boy emulator reset");
}

void Gameboy::step() {
    cpu.step();
}

void Gameboy::runFrame() {
    for (int i = 0; i < 100; ++i) {
        if (!cpu.isHalted()) {
            step();
        }
    }
}

void Gameboy::setButton(int button, bool pressed) {
    // TODO: Joypad implementation
}

const uint8_t* Gameboy::getMemoryPtr() const {
    return memory.getMemoryPtr();
}