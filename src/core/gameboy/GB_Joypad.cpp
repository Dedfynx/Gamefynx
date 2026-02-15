#include "core/gameboy/GB_Joypad.h"
#include "core/gameboy/GB_MMU.h"
#include "utils/Logger.h"

GB_Joypad::GB_Joypad(GB_MMU& mem) : mmu(mem) {
    reset();
}

void GB_Joypad::reset() {
    buttonStates = 0xFF;
    LOG_DEBUG("GB Joypad reset");
}

void GB_Joypad::setButton(int button, bool pressed) {
    if (button < 0 || button > 7) return;

    if (pressed) {
        buttonStates &= ~(1 << button);
        
        uint8_t IF = mmu.read(0xFF0F);
        mmu.write(0xFF0F, IF | 0x10);
    } else {
        buttonStates |= (1 << button);
    }
}

void GB_Joypad::update() {
    uint8_t joyp = mmu.read(0xFF00);
    uint8_t result = 0xCF;
    
    if ((joyp & 0x20) == 0) {
        result &= ~0x20;
        if ((buttonStates & (1 << Button::A)) == 0)      result &= ~0x01;
        if ((buttonStates & (1 << Button::B)) == 0)      result &= ~0x02;
        if ((buttonStates & (1 << Button::SELECT)) == 0) result &= ~0x04;
        if ((buttonStates & (1 << Button::START)) == 0)  result &= ~0x08;
    }
    
    if ((joyp & 0x10) == 0) {
        result &= ~0x10;
        if ((buttonStates & (1 << Button::RIGHT)) == 0) result &= ~0x01;
        if ((buttonStates & (1 << Button::LEFT)) == 0)  result &= ~0x02;
        if ((buttonStates & (1 << Button::UP)) == 0)    result &= ~0x04;
        if ((buttonStates & (1 << Button::DOWN)) == 0)  result &= ~0x08;
    }
    
    mmu.write(0xFF00, result);
}