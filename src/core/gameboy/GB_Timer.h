#pragma once
#include "common/Types.h"

class GB_MMU;

class GB_Timer {
public:
    GB_Timer(GB_MMU& mmu);

    void reset();
    void step(int cycles);

    uint8_t getDIV() const {
        return (internalCounter >> 8) & 0xFF;
    }

    void resetDIV();
    void writeTAC(uint8_t value);

private:
    GB_MMU& mmu;

    uint16_t internalCounter = 0;
    uint16_t prevInternalCounter = 0;

    bool getTimerBit() const;

    void updateTIMA();
    int getMultiplierBit() const;
};