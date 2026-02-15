#include "core/chip8/Chip8.h"
#include "utils/Logger.h"

#include <fstream>

static const uint8_t CHIP8_FONTSET[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

Chip8::Chip8() {
    memory.fill(0);
    V.fill(0);
    stack.fill(0);
    display.fill(0);
    keypad.fill(0);
    
    I = 0;
    pc = 0x200;
    sp = 0;
    delay_timer = 0;
    sound_timer = 0;
    draw_flag = false;
    rom_loaded = false;
    
    loadFontset();
    LOG_INFO("CHIP-8 emulator initialized");
}

bool Chip8::loadROM(const std::string& path) {
    LOG_INFO("Loading ROM: {}", path);
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        LOG_ERROR("Failed to open ROM file: {}", path);
        return false;
    }
    
    size_t size = file.tellg();
    file.seekg(0);
    
    LOG_DEBUG("ROM size: {} bytes", size);
    if (size > 4096 - 0x200) {
        LOG_ERROR("ROM too large: {} bytes (max: {})", size, 4096 - 0x200);
        return false; 
    }
    
    file.read(reinterpret_cast<char*>(&memory[0x200]), size);
    LOG_INFO("ROM loaded successfully");

    //repeat 
    V.fill(0);
    stack.fill(0);
    display.fill(0);
    I = 0;
    pc = 0x200;
    sp = 0;
    delay_timer = 0;
    sound_timer = 0;
    draw_flag = false;
    rom_loaded = true;
    return true;
}

void Chip8::reset() {
    if (!rom_loaded) {
        LOG_WARN("Reset called but no ROM is loaded");
        return;
    }

    LOG_DEBUG("Resetting emulator state");

    V.fill(0);
    stack.fill(0);
    display.fill(0);
    keypad.fill(0);
    
    I = 0;
    pc = 0x200;
    sp = 0;
    
    delay_timer = 0;
    sound_timer = 0;
    draw_flag = false;
}

void Chip8::step() {
    // Fetch opcode (2 bytes, big-endian)
    uint16_t opcode = (memory[pc] << 8) | memory[pc + 1];
    
    // Décode et exécute
    executeOpcode(opcode);
    
    // Update timers
    updateTimers();
}

void Chip8::runFrame() {
    // TODO: run ~60 instructions (CHIP-8 ~540Hz, 60fps = ~9 cycles/frame)
    for (int i = 0; i < 9; ++i) {
        step();
    }
}

const uint8_t* Chip8::getFramebuffer() const {
    return display.data();
}

void Chip8::setButton(int button, bool pressed) {
    if (button >= 0 && button < 16) {
        keypad[button] = pressed ? 1 : 0;
    }
}

void Chip8::loadFontset() {
    // Font sprites vont de 0x000 à 0x04F
    for (int i = 0; i < 80; ++i) {
        memory[i] = CHIP8_FONTSET[i];
    }
}

void Chip8::updateTimers() {
    if (delay_timer > 0) {
        --delay_timer;
    }
    
    if (sound_timer > 0) {
        if (audio && !audio->isPlaying()) {
           audio->playBeep();
        }
        --sound_timer;
        if (audio && sound_timer == 0)
        {
            audio->stopBeep();
        }
    }
}

void Chip8::executeOpcode(uint16_t opcode) {
    // Extrait le premier nibble (4 bits) pour router
    uint8_t first_nibble = (opcode & 0xF000) >> 12;
    
    switch (first_nibble) {
        case 0x0: op_0xxx(opcode); break;
        case 0x1: op_1xxx(opcode); break;
        case 0x2: op_2xxx(opcode); break;
        case 0x3: op_3xxx(opcode); break;
        case 0x4: op_4xxx(opcode); break;
        case 0x5: op_5xxx(opcode); break;
        case 0x6: op_6xxx(opcode); break;
        case 0x7: op_7xxx(opcode); break;
        case 0x8: op_8xxx(opcode); break;
        case 0x9: op_9xxx(opcode); break;
        case 0xA: op_Axxx(opcode); break;
        case 0xB: op_Bxxx(opcode); break;
        case 0xC: op_Cxxx(opcode); break;
        case 0xD: op_Dxxx(opcode); break;
        case 0xE: op_Exxx(opcode); break;
        case 0xF: op_Fxxx(opcode); break;
    }
}

#pragma region Opcode
void Chip8::op_0xxx(uint16_t opcode) {
    switch (opcode & 0x00FF) {
        case 0xE0:  // 00E0 - Clear screen
            display.fill(0);
            draw_flag = true;
            pc += 2;
            break;
            
        case 0xEE:  // 00EE - Return from subroutine
            --sp;
            pc = stack[sp];
            pc += 2;
            break;
            
        default:
            // 0NNN - Call RCA 1802 program (ignoré dans les émulateurs modernes)
            pc += 2;
            break;
    }
}

void Chip8::op_1xxx(uint16_t opcode) {
    // 1NNN - Jump to address NNN
    uint16_t address = opcode & 0x0FFF;
    pc = address;
}

void Chip8::op_2xxx(uint16_t opcode) {
    // 2NNN - Call subroutine at NNN
    stack[sp] = pc;
    ++sp;
    pc = opcode & 0x0FFF;
}

void Chip8::op_3xxx(uint16_t opcode) {
    // 3XNN - Skip next instruction if VX == NN
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t nn = opcode & 0x00FF;
    
    if (V[x] == nn) {
        pc += 4;
    } else {
        pc += 2;
    }
}

void Chip8::op_4xxx(uint16_t opcode) {
    // 4XNN - Skip next instruction if VX != NN
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t nn = opcode & 0x00FF;
    
    if (V[x] != nn) {
        pc += 4;
    } else {
        pc += 2;
    }
}

void Chip8::op_5xxx(uint16_t opcode) {
    // 5XY0 - Skip next instruction if VX == VY
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    
    if (V[x] == V[y]) {
        pc += 4;
    } else {
        pc += 2;
    }
}

void Chip8::op_6xxx(uint16_t opcode) {
    // 6XNN - Set VX = NN
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t nn = opcode & 0x00FF;
    
    V[x] = nn;
    pc += 2;
}

void Chip8::op_7xxx(uint16_t opcode) {
    // 7XNN - Add NN to VX (carry flag not changed)
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t nn = opcode & 0x00FF;
    
    V[x] += nn;
    pc += 2;
}

void Chip8::op_8xxx(uint16_t opcode) {
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    uint8_t n = opcode & 0x000F;
    
    switch (n) {
        case 0x0:  // 8XY0 - VX = VY
            V[x] = V[y];
            break;
            
        case 0x1:  // 8XY1 - VX |= VY
            V[x] |= V[y];
            break;
            
        case 0x2:  // 8XY2 - VX &= VY
            V[x] &= V[y];
            break;
            
        case 0x3:  // 8XY3 - VX ^= VY
            V[x] ^= V[y];
            break;
            
        case 0x4:  // 8XY4 - VX += VY, VF = carry
            {
                uint16_t sum = V[x] + V[y];
                V[0xF] = (sum > 0xFF) ? 1 : 0;
                V[x] = sum & 0xFF;
            }
            break;
            
        case 0x5:  // 8XY5 - VX -= VY, VF = NOT borrow
            V[0xF] = (V[x] >= V[y]) ? 1 : 0;
            V[x] -= V[y];
            break;
            
        case 0x6:  // 8XY6 - VX >>= 1, VF = bit shifted out
            V[0xF] = V[x] & 0x1;
            V[x] >>= 1;
            break;
            
        case 0x7:  // 8XY7 - VX = VY - VX, VF = NOT borrow
            V[0xF] = (V[y] >= V[x]) ? 1 : 0;
            V[x] = V[y] - V[x];
            break;
            
        case 0xE:  // 8XYE - VX <<= 1, VF = bit shifted out
            V[0xF] = (V[x] & 0x80) >> 7;
            V[x] <<= 1;
            break;
    }
    
    pc += 2;
}

void Chip8::op_9xxx(uint16_t opcode) {
    // 9XY0 - Skip next instruction if VX != VY
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    
    if (V[x] != V[y]) {
        pc += 4;
    } else {
        pc += 2;
    }
}

void Chip8::op_Axxx(uint16_t opcode) {
    // ANNN - Set I = NNN
    I = opcode & 0x0FFF;
    pc += 2;
}

void Chip8::op_Bxxx(uint16_t opcode) {
    // BNNN - Jump to address NNN + V0
    uint16_t address = opcode & 0x0FFF;
    pc = address + V[0];
}

void Chip8::op_Cxxx(uint16_t opcode) {
    // CXNN - Set VX = random byte AND NN
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t nn = opcode & 0x00FF;
    
    V[x] = (rand() % 256) & nn;
    pc += 2;
}

void Chip8::op_Dxxx(uint16_t opcode) {
    // DXYN - Draw sprite at (VX, VY) with height N
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    uint8_t height = opcode & 0x000F;
    
    uint8_t x_pos = V[x] % 64;
    uint8_t y_pos = V[y] % 32;
    
    V[0xF] = 0;  // Reset collision flag
    
    for (int row = 0; row < height; ++row) {
        uint8_t sprite_byte = memory[I + row];
        
        for (int col = 0; col < 8; ++col) {
            uint8_t sprite_pixel = sprite_byte & (0x80 >> col);
            
            if (sprite_pixel != 0) {
                int screen_x = (x_pos + col) % 64;
                int screen_y = (y_pos + row) % 32;
                int screen_index = screen_y * 64 + screen_x;
                
                // XOR le pixel
                if (display[screen_index] == 1) {
                    V[0xF] = 1;  // Collision détectée
                }
                display[screen_index] ^= 1;
            }
        }
    }
    
    draw_flag = true;
    pc += 2;
}

void Chip8::op_Exxx(uint16_t opcode) {
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t nn = opcode & 0x00FF;
    
    switch (nn) {
        case 0x9E:  // EX9E - Skip if key VX is pressed
            if (keypad[V[x]] != 0) {
                pc += 4;
            } else {
                pc += 2;
            }
            break;
            
        case 0xA1:  // EXA1 - Skip if key VX is NOT pressed
            if (keypad[V[x]] == 0) {
                pc += 4;
            } else {
                pc += 2;
            }
            break;
    }
}

void Chip8::op_Fxxx(uint16_t opcode) {
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t nn = opcode & 0x00FF;
    
    switch (nn) {
        case 0x07:  // FX07 - VX = delay_timer
            V[x] = delay_timer;
            break;
            
        case 0x0A:  // FX0A - Wait for key press, store in VX
            {
                bool key_pressed = false;
                for (int i = 0; i < 16; ++i) {
                    if (keypad[i] != 0) {
                        V[x] = i;
                        key_pressed = true;
                        break;
                    }
                }
                if (!key_pressed) {
                    return;  // Ne pas avancer PC, on attend
                }
            }
            break;
            
        case 0x15:  // FX15 - delay_timer = VX
            delay_timer = V[x];
            break;
            
        case 0x18:  // FX18 - sound_timer = VX
            sound_timer = V[x];
            break;
            
        case 0x1E:  // FX1E - I += VX
            I += V[x];
            break;
            
        case 0x29:  // FX29 - I = location of sprite for digit VX
            I = V[x] * 5;  // Chaque sprite de font = 5 bytes
            break;
            
        case 0x33:  // FX33 - Store BCD representation of VX in I, I+1, I+2
            memory[I]     = V[x] / 100;
            memory[I + 1] = (V[x] / 10) % 10;
            memory[I + 2] = V[x] % 10;
            break;
            
        case 0x55:  // FX55 - Store V0 to VX in memory starting at I
            for (int i = 0; i <= x; ++i) {
                memory[I + i] = V[i];
            }
            break;
            
        case 0x65:  // FX65 - Read V0 to VX from memory starting at I
            for (int i = 0; i <= x; ++i) {
                V[i] = memory[I + i];
            }
            break;
    }
    
    pc += 2;
}
#pragma endregion