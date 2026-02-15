//
// Created by Dedfynx on 11/02/2026.
//

#include "Gameboy.h"
#include "utils/Logger.h"
#include "config/EmulatorConfig.h"

Gameboy::Gameboy() : cpu(memory), ppu(memory), joypad(memory) {
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
    romLoaded = true;

    return true;
}

void Gameboy::reset() {
    cpu.reset();
    ppu.reset();
    joypad.reset();
    framebuffer.fill(0xFF);
    LOG_DEBUG("Game Boy emulator reset");
}

void Gameboy::step() {
    cpu.step();
}

void Gameboy::runFrame() {
    if (!romLoaded) return;

    cpu.resetCycles();

    while (cpu.getCycles() < Config::GB_CYCLES_PER_FRAME) {
        int cyclesBefore = cpu.getCycles();

        if (!cpu.isHalted()) {
            cpu.step();
        } else {
            cpu.addCycles(4);
        }

        int cyclesExecuted = cpu.getCycles() - cyclesBefore;
        ppu.step(cyclesExecuted);
    }

    joypad.update();

    if (ppu.isFrameReady()) {
        std::memcpy(framebuffer.data(), ppu.getFramebuffer(), framebuffer.size());
        ppu.clearFrameReady();
    }
}

void Gameboy::setButton(int button, bool pressed) {
    joypad.setButton(button, pressed);
}

const uint8_t* Gameboy::getMemoryPtr() const {
    return memory.getMemoryPtr();
}