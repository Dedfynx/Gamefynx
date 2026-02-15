//
// Created by Dedfynx on 11/02/2026.
//
#pragma once

#include "common/types.h"

class GB_MMU;

class GB_CPU
{
public:
    explicit GB_CPU(GB_MMU& mmu);
    ~GB_CPU() = default;

    void reset();
    void step();
    void execute(uint8_t opcode);
    void executeCB(uint8_t opcode);

    int getCycles() const { return cycles; }
    bool isHalted() const { return halted; }

    void resetCycles() { cycles = 0; }
    void addCycles(int c) { cycles += c; }

    void handleInterrupts(GB_MMU& mmu);

    bool isIME() const { return ime; }
    void setIME(bool value) { ime = value; }

    // Registres 16 bits
    uint16_t af = 0, bc = 0, de = 0, hl = 0, sp = 0, pc = 0;

    // Registres 8 bits
    uint8_t& a = reinterpret_cast<uint8_t*>(&af)[1];
    uint8_t& f = reinterpret_cast<uint8_t*>(&af)[0];
    uint8_t& b = reinterpret_cast<uint8_t*>(&bc)[1];
    uint8_t& c = reinterpret_cast<uint8_t*>(&bc)[0];
    uint8_t& d = reinterpret_cast<uint8_t*>(&de)[1];
    uint8_t& e = reinterpret_cast<uint8_t*>(&de)[0];
    uint8_t& h = reinterpret_cast<uint8_t*>(&hl)[1];
    uint8_t& l = reinterpret_cast<uint8_t*>(&hl)[0];

private:
    GB_MMU& mmu;

    bool ime = false;
    bool halted = false;
    int cycles = 0;

    enum Flags {
        Z_FLAG = 0x80,
        N_FLAG = 0x40,
        H_FLAG = 0x20,
        C_FLAG = 0x10
    };

    inline bool getFlag(Flags flag) const { return f & flag; }
    inline void setFlag(Flags flag, bool value) {
        if (value) {
            f |= static_cast<uint8_t>(flag);
        } else {
            f &= ~static_cast<uint8_t>(flag);
        }
        f &= 0xF0;
    }
};