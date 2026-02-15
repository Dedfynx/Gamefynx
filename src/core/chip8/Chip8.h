#pragma once
#include "common/EmulatorInterface.h"
#include "common/types.h"
#include "utils/Audio.h"
#include <array>

class Chip8 : public IEmulator {
public:
    Chip8();
    
    bool loadROM(const std::string& path) override;
    void reset() override;
    void step() override;
    void runFrame() override;
    
    const uint8_t* getFramebuffer() const override;
    int getScreenWidth() const override { return 64; }
    int getScreenHeight() const override { return 32; }
    
    void setButton(int button, bool pressed) override;
    std::string getArchName() const override { return "CHIP-8"; }
    uint16_t getPC() const override{ return pc; }
    const uint8_t* getMemoryPtr() const override{ return memory.data();};
    size_t getMemorySize() const override{ return memory.size(); };

    uint16_t getI() const { return I; }
    uint8_t getV(int reg) const { return V[reg]; }
    const std::array<uint8_t, 16>& getKeypad() const { return keypad; }

    void setAudio(Audio* audio) { this->audio = audio; }


private:
    // Specs CHIP-8
    std::array<uint8_t, 4096> memory{};       // 4K RAM
    std::array<uint8_t, 16> V{};              // 16 registres
    uint16_t I = 0;                           // Index register
    uint16_t pc = 0x200;                      // Program counter
    
    std::array<uint8_t, 64*32> display{};     // 64x32 monochrome
    bool draw_flag = false;
    std::array<uint8_t, 16> keypad{};         // 16 touches
    
    
    uint8_t delay_timer = 0;
    uint8_t sound_timer = 0;

    Audio* audio = nullptr;
    
    std::array<uint16_t, 16> stack{};
    uint8_t sp = 0;

    bool rom_loaded = false;
        
    void executeOpcode(uint16_t opcode);

    void op_0xxx(uint16_t opcode);  // 00E0, 00EE
    void op_1xxx(uint16_t opcode);  // 1NNN - Jump
    void op_2xxx(uint16_t opcode);  // 2NNN - Call
    void op_3xxx(uint16_t opcode);  // 3XNN - Skip if VX == NN
    void op_4xxx(uint16_t opcode);  // 4XNN - Skip if VX != NN
    void op_5xxx(uint16_t opcode);  // 5XY0 - Skip if VX == VY
    void op_6xxx(uint16_t opcode);  // 6XNN - Set VX = NN
    void op_7xxx(uint16_t opcode);  // 7XNN - VX += NN
    void op_8xxx(uint16_t opcode);  // Opérations arithmétiques/logiques
    void op_9xxx(uint16_t opcode);  // 9XY0 - Skip if VX != VY
    void op_Axxx(uint16_t opcode);  // ANNN - Set I = NNN
    void op_Bxxx(uint16_t opcode);  // BNNN - Jump to NNN + V0
    void op_Cxxx(uint16_t opcode);  // CXNN - VX = rand() & NN
    void op_Dxxx(uint16_t opcode);  // DXYN - Draw sprite
    void op_Exxx(uint16_t opcode);  // EX9E, EXA1 - Input
    void op_Fxxx(uint16_t opcode);  // Timers, memory, etc.

    void loadFontset();
    void updateTimers();
};