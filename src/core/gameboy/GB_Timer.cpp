#include "core/gameboy/GB_Timer.h"
#include "core/gameboy/GB_MMU.h"

GB_Timer::GB_Timer(GB_MMU& mem) : mmu(mem) {
    reset();
}

void GB_Timer::reset() {
    internalCounter = 0;
    prevInternalCounter = 0;
}

void GB_Timer::resetDIV() {
    // ⚡ Quand DIV est reset, check TIMA avant
    updateTIMA();
    prevInternalCounter = internalCounter;
    internalCounter = 0;
}

int GB_Timer::getMultiplierBit() const {
    uint8_t tac = mmu.read(0xFF07);

    // ⚡ Bits à checker selon TAC
    switch (tac & 0x03) {
    case 0: return 9;   // 1024 cycles (bit 9)
    case 1: return 3;   // 16 cycles (bit 3)
    case 2: return 5;   // 64 cycles (bit 5)
    case 3: return 7;   // 256 cycles (bit 7)
    }
    return 9;
}

bool GB_Timer::getTimerBit() const {
    uint8_t tac = mmu.read(0xFF07);

    // Timer désactivé → bit = 0
    if ((tac & 0x04) == 0) return false;

    int bit = getMultiplierBit();
    return (internalCounter & (1 << bit)) != 0;
}

void GB_Timer::writeTAC(uint8_t value) {
    uint8_t oldTAC = mmu.read(0xFF07);

    bool oldEnabled = (oldTAC & 0x04) != 0;
    bool newEnabled = (value & 0x04) != 0;

    // ⚡ Si le timer était activé, check falling edge
    if (oldEnabled) {
        bool oldBit = getTimerBit();

        // Écrit la nouvelle valeur
        mmu.directWriteTAC(value);

        bool newBit = getTimerBit();

        // Falling edge → Incrémente TIMA
        if (oldBit && !newBit) {
            uint8_t tima = mmu.read(0xFF05);

            if (tima == 0xFF) {
                uint8_t tma = mmu.read(0xFF06);
                mmu.write(0xFF05, tma);

                uint8_t IF = mmu.read(0xFF0F);
                mmu.write(0xFF0F, IF | 0x04);
            } else {
                mmu.write(0xFF05, tima + 1);
            }
        }
    } else {
        // Timer était désactivé, juste écrire
        mmu.directWriteTAC(value);
    }
}

void GB_Timer::updateTIMA() {
    uint8_t tac = mmu.read(0xFF07);

    if ((tac & 0x04) == 0) return;

    int bit = getMultiplierBit();

    bool prevBit = (prevInternalCounter & (1 << bit)) != 0;
    bool newBit = (internalCounter & (1 << bit)) != 0;

    if (prevBit && !newBit) {
        uint8_t tima = mmu.read(0xFF05);

        if (tima == 0xFF) {
            uint8_t tma = mmu.read(0xFF06);
            mmu.write(0xFF05, tma);

            uint8_t IF = mmu.read(0xFF0F);
            mmu.write(0xFF0F, IF | 0x04);
        } else {
            mmu.write(0xFF05, tima + 1);
        }
    }
}

void GB_Timer::step(int cycles) {
    // Sauvegarde l'ancien compteur
    prevInternalCounter = internalCounter;

    // Incrémente le compteur interne
    internalCounter += cycles;

    // Update TIMA basé sur les falling edges
    updateTIMA();
}