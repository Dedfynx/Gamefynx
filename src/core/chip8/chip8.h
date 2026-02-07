#pragma once
#include "common/emulator_interface.h"
#include "common/types.h"
#include <array>

class Chip8Emulator : public IEmulator {
public:
    Chip8Emulator();
    
    // Interface IEmulator
    bool loadROM(const std::string& path) override;
    void reset() override;
    void step() override;
    void runFrame() override;
    
    const uint8_t* getFramebuffer() const override;
    int getScreenWidth() const override { return 64; }
    int getScreenHeight() const override { return 32; }
    
    void setButton(int button, bool pressed) override;
    std::string getArchName() const override { return "CHIP-8"; }
    
private:
    // Specs CHIP-8
    std::array<uint8_t, 4096> memory{};       // 4K RAM
    std::array<uint8_t, 16> V{};              // 16 registres
    uint16_t I = 0;                           // Index register
    uint16_t pc = 0x200;                      // Program counter
    
    std::array<uint8_t, 64*32> display{};     // 64x32 monochrome
    std::array<uint8_t, 16> keypad{};         // 16 touches
    
    uint8_t delay_timer = 0;
    uint8_t sound_timer = 0;
    
    std::array<uint16_t, 16> stack{};
    uint8_t sp = 0;
    
    // Helpers
    void executeOpcode(uint16_t opcode);
};