//
// Created by Dedfynx on 11/02/2026.
//

#pragma once
#include "common/emulator_interface.h"
#include "common/types.h"
#include "GB_MMU.h"
#include "GB_CPU.h"

class Gameboy : public IEmulator
{
    public:
    Gameboy();
    ~Gameboy() = default;

    bool loadROM(const std::string& path) override;
    void reset() override;
    void step() override;
    void runFrame() override;

    const uint8_t* getFramebuffer() const override { return framebuffer.data(); }
    int getScreenWidth() const override { return 160; }
    int getScreenHeight() const override { return 144; }

    void setButton(int button, bool pressed) override;
    std::string getArchName() const override { return "Game Boy"; }

    const uint8_t* getMemoryPtr() const override;
    size_t getMemorySize() const override { return 0x10000; }  // 64KB
    uint16_t getPC() const override {return cpu.pc;}

    const GB_CPU& getCPU() const { return cpu; }
    const GB_MMU& getMemory() const { return memory; }

private:
    GB_MMU memory;
    GB_CPU cpu;

    std::array<uint8_t, 160 * 144 * 4> framebuffer{};
    bool rom_loaded = false;
};
