#pragma once
#include "common/Types.h"

class GB_MMU;

class GB_Joypad {
public:
    GB_Joypad(GB_MMU& mmu);

    void reset();
    void update();
    void setButton(int button, bool pressed);

    enum Button {
        A      = 0,
        B      = 1,
        SELECT = 2,
        START  = 3,
        RIGHT  = 4,
        LEFT   = 5,
        UP     = 6,
        DOWN   = 7
    };

private:
    GB_MMU& mmu;
    uint8_t buttonStates = 0xFF;
};