#include "core/gameboy/GB_CPU.h"
#include "core/gameboy/GB_MMU.h"
#include "utils/Logger.h"

GB_CPU::GB_CPU(GB_MMU& mmu) : mmu(mmu) {
    reset();
}

void GB_CPU::reset() {
    // Post BootRom
    af = 0x01B0;
    bc = 0x0013;
    de = 0x00D8;
    hl = 0x014D;
    sp = 0xFFFE;
    pc = 0x0100;

    f &=  0xF0; //Force le masque sur les flags

    ime = false;
    halted = false;
    cycles = 0;

    LOG_INFO("GB CPU reset - PC: {:#06x}", pc);
}

void GB_CPU::step() {
    if (halted) {
        cycles += 4;
        return;
    }

    uint8_t opcode = mmu.read(pc++);
    execute(opcode);

    // Debug serial output (Blargg tests)
    mmu.dbg_serial();
}

void GB_CPU::execute(uint8_t opcode) {
    static int count = 0;
    if (count++ < 100) {  // Log les 100 premières instructions
        LOG_DEBUG("PC: {:#06x} | OP: {:#04x} | A: {:#04x} F: {:#04x} BC: {:#06x} HL: {:#06x}",
                  pc - 1, opcode, a, f, bc, hl);
    }

    switch (opcode) {
        case 0x00: cycles += 4; break; //NOP

        case 0x01: c = mmu.read(pc++); b = mmu.read(pc++); cycles += 12; break;  // LD BC, nn
        case 0x11: e = mmu.read(pc++); d = mmu.read(pc++); cycles += 12; break;  // LD DE, nn
        case 0x21: l = mmu.read(pc++); h = mmu.read(pc++); cycles += 12; break;  // LD HL, nn
        case 0x31: sp = mmu.read(pc++) | (mmu.read(pc++) << 8); cycles += 12; break;  // LD SP, nn

        case 0x06: b = mmu.read(pc++); cycles += 8; break;  // LD B, n
        case 0x0E: c = mmu.read(pc++); cycles += 8; break;  // LD C, n
        case 0x16: d = mmu.read(pc++); cycles += 8; break;  // LD D, n
        case 0x1E: e = mmu.read(pc++); cycles += 8; break;  // LD E, n
        case 0x26: h = mmu.read(pc++); cycles += 8; break;  // LD H, n
        case 0x2E: l = mmu.read(pc++); cycles += 8; break;  // LD L, n
        case 0x3E: a = mmu.read(pc++); cycles += 8; break;  // LD A, n

        case 0x40: cycles += 4; break;  // LD B, B
        case 0x41: b = c; cycles += 4; break;  // LD B, C
        case 0x42: b = d; cycles += 4; break;  // LD B, D
        case 0x43: b = e; cycles += 4; break;  // LD B, E
        case 0x44: b = h; cycles += 4; break;  // LD B, H
        case 0x45: b = l; cycles += 4; break;  // LD B, L
        case 0x46: b = mmu.read(hl); cycles += 8; break;  // LD B, (HL)
        case 0x47: b = a; cycles += 4; break;  // LD B, A

        case 0x48: c = b; cycles += 4; break;  // LD C, B
        case 0x49: cycles += 4; break;  // LD C, C
        case 0x4A: c = d; cycles += 4; break;  // LD C, D
        case 0x4B: c = e; cycles += 4; break;  // LD C, E
        case 0x4C: c = h; cycles += 4; break;  // LD C, H
        case 0x4D: c = l; cycles += 4; break;  // LD C, L
        case 0x4E: c = mmu.read(hl); cycles += 8; break;  // LD C, (HL)
        case 0x4F: c = a; cycles += 4; break;  // LD C, A

        case 0x50: d = b; cycles += 4; break;  // LD D, B
        case 0x51: d = c; cycles += 4; break;  // LD D, C
        case 0x52: cycles += 4; break;  // LD D, D
        case 0x53: d = e; cycles += 4; break;  // LD D, E
        case 0x54: d = h; cycles += 4; break;  // LD D, H
        case 0x55: d = l; cycles += 4; break;  // LD D, L
        case 0x56: d = mmu.read(hl); cycles += 8; break;  // LD D, (HL)
        case 0x57: d = a; cycles += 4; break;  // LD D, A

        case 0x58: e = b; cycles += 4; break;  // LD E, B
        case 0x59: e = c; cycles += 4; break;  // LD E, C
        case 0x5A: e = d; cycles += 4; break;  // LD E, D
        case 0x5B: cycles += 4; break;  // LD E, E
        case 0x5C: e = h; cycles += 4; break;  // LD E, H
        case 0x5D: e = l; cycles += 4; break;  // LD E, L
        case 0x5E: e = mmu.read(hl); cycles += 8; break;  // LD E, (HL)
        case 0x5F: e = a; cycles += 4; break;  // LD E, A

        case 0x60: h = b; cycles += 4; break;  // LD H, B
        case 0x61: h = c; cycles += 4; break;  // LD H, C
        case 0x62: h = d; cycles += 4; break;  // LD H, D
        case 0x63: h = e; cycles += 4; break;  // LD H, E
        case 0x64: cycles += 4; break;  // LD H, H
        case 0x65: h = l; cycles += 4; break;  // LD H, L
        case 0x66: h = mmu.read(hl); cycles += 8; break;  // LD H, (HL)
        case 0x67: h = a; cycles += 4; break;  // LD H, A

        case 0x68: l = b; cycles += 4; break;  // LD L, B
        case 0x69: l = c; cycles += 4; break;  // LD L, C
        case 0x6A: l = d; cycles += 4; break;  // LD L, D
        case 0x6B: l = e; cycles += 4; break;  // LD L, E
        case 0x6C: l = h; cycles += 4; break;  // LD L, H
        case 0x6D: cycles += 4; break;  // LD L, L
        case 0x6E: l = mmu.read(hl); cycles += 8; break;  // LD L, (HL)
        case 0x6F: l = a; cycles += 4; break;  // LD L, A

        case 0x70: mmu.write(hl, b); cycles += 8; break;  // LD (HL), B
        case 0x71: mmu.write(hl, c); cycles += 8; break;  // LD (HL), C
        case 0x72: mmu.write(hl, d); cycles += 8; break;  // LD (HL), D
        case 0x73: mmu.write(hl, e); cycles += 8; break;  // LD (HL), E
        case 0x74: mmu.write(hl, h); cycles += 8; break;  // LD (HL), H
        case 0x75: mmu.write(hl, l); cycles += 8; break;  // LD (HL), L
        case 0x36: mmu.write(hl, mmu.read(pc++)); cycles += 12; break;  // LD (HL), n
        case 0x77: mmu.write(hl, a); cycles += 8; break;  // LD (HL), A

        case 0x78: a = b; cycles += 4; break;  // LD A, B
        case 0x79: a = c; cycles += 4; break;  // LD A, C
        case 0x7A: a = d; cycles += 4; break;  // LD A, D
        case 0x7B: a = e; cycles += 4; break;  // LD A, E
        case 0x7C: a = h; cycles += 4; break;  // LD A, H
        case 0x7D: a = l; cycles += 4; break;  // LD A, L
        case 0x7E: a = mmu.read(hl); cycles += 8; break;  // LD A, (HL)
        case 0x7F: cycles += 4; break;  // LD A, A

        case 0x0A: a = mmu.read(bc); cycles += 8; break;  // LD A, (BC)
        case 0x1A: a = mmu.read(de); cycles += 8; break;  // LD A, (DE)
        case 0xFA: { uint16_t addr = mmu.read(pc++) | (mmu.read(pc++) << 8); a = mmu.read(addr); cycles += 16; } break;  // LD A, (nn)

        case 0x02: mmu.write(bc, a); cycles += 8; break;  // LD (BC), A
        case 0x12: mmu.write(de, a); cycles += 8; break;  // LD (DE), A
        case 0xEA: { uint16_t addr = mmu.read(pc++) | (mmu.read(pc++) << 8); mmu.write(addr, a); cycles += 16; } break;  // LD (nn), A

        case 0xE0: mmu.write(0xFF00 + mmu.read(pc++), a); cycles += 12; break;  // LDH (n), A
        case 0xF0: a = mmu.read(0xFF00 + mmu.read(pc++)); cycles += 12; break;  // LDH A, (n)
        case 0xE2: mmu.write(0xFF00 + c, a); cycles += 8; break;  // LD (C), A
        case 0xF2: a = mmu.read(0xFF00 + c); cycles += 8; break;  // LD A, (C)

        case 0xF8: {
            int8_t offset = static_cast<int8_t>(mmu.read(pc++));
            uint16_t result = sp + offset;
            hl = result;
            setFlag(Z_FLAG, false);
            setFlag(N_FLAG, false);
            setFlag(H_FLAG, ((sp & 0x0F) + (offset & 0x0F)) > 0x0F);
            setFlag(C_FLAG, ((sp & 0xFF) + (offset & 0xFF)) > 0xFF);
            cycles += 12;
        } break;

        case 0xF9: sp = hl; cycles += 8; break;

        case 0xC5: mmu.write(--sp, b); mmu.write(--sp, c); cycles += 16; break;  // PUSH BC
        case 0xD5: mmu.write(--sp, d); mmu.write(--sp, e); cycles += 16; break;  // PUSH DE
        case 0xE5: mmu.write(--sp, h); mmu.write(--sp, l); cycles += 16; break;  // PUSH HL
        case 0xF5: mmu.write(--sp, a); mmu.write(--sp, f & 0xF0); cycles += 16; break;  // PUSH AF

        case 0xC1: c = mmu.read(sp++); b = mmu.read(sp++); cycles += 12; break;  // POP BC
        case 0xD1: e = mmu.read(sp++); d = mmu.read(sp++); cycles += 12; break;  // POP DE
        case 0xE1: l = mmu.read(sp++); h = mmu.read(sp++); cycles += 12; break;  // POP HL
        case 0xF1: f = mmu.read(sp++) & 0xF0; a = mmu.read(sp++); cycles += 12; break;  // POP AF

        // Add
        case 0x80: { uint8_t val = b; uint16_t result = a + val; setFlag(Z_FLAG, (result & 0xFF) == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, ((a & 0x0F) + (val & 0x0F)) > 0x0F); setFlag(C_FLAG, result > 0xFF); a = result & 0xFF; cycles += 4; } break;
        case 0x81: { uint8_t val = c; uint16_t result = a + val; setFlag(Z_FLAG, (result & 0xFF) == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, ((a & 0x0F) + (val & 0x0F)) > 0x0F); setFlag(C_FLAG, result > 0xFF); a = result & 0xFF; cycles += 4; } break;
        case 0x82: { uint8_t val = d; uint16_t result = a + val; setFlag(Z_FLAG, (result & 0xFF) == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, ((a & 0x0F) + (val & 0x0F)) > 0x0F); setFlag(C_FLAG, result > 0xFF); a = result & 0xFF; cycles += 4; } break;
        case 0x83: { uint8_t val = e; uint16_t result = a + val; setFlag(Z_FLAG, (result & 0xFF) == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, ((a & 0x0F) + (val & 0x0F)) > 0x0F); setFlag(C_FLAG, result > 0xFF); a = result & 0xFF; cycles += 4; } break;
        case 0x84: { uint8_t val = h; uint16_t result = a + val; setFlag(Z_FLAG, (result & 0xFF) == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, ((a & 0x0F) + (val & 0x0F)) > 0x0F); setFlag(C_FLAG, result > 0xFF); a = result & 0xFF; cycles += 4; } break;
        case 0x85: { uint8_t val = l; uint16_t result = a + val; setFlag(Z_FLAG, (result & 0xFF) == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, ((a & 0x0F) + (val & 0x0F)) > 0x0F); setFlag(C_FLAG, result > 0xFF); a = result & 0xFF; cycles += 4; } break;
        case 0x86: { uint8_t val = mmu.read(hl); uint16_t result = a + val; setFlag(Z_FLAG, (result & 0xFF) == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, ((a & 0x0F) + (val & 0x0F)) > 0x0F); setFlag(C_FLAG, result > 0xFF); a = result & 0xFF; cycles += 8; } break;
        case 0x87: { uint16_t result = a + a; setFlag(Z_FLAG, (result & 0xFF) == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, ((a & 0x0F) + (a & 0x0F)) > 0x0F); setFlag(C_FLAG, result > 0xFF); a = result & 0xFF; cycles += 4; } break;
        case 0xC6: { uint8_t val = mmu.read(pc++); uint16_t result = a + val; setFlag(Z_FLAG, (result & 0xFF) == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, ((a & 0x0F) + (val & 0x0F)) > 0x0F); setFlag(C_FLAG, result > 0xFF); a = result & 0xFF; cycles += 8; } break;

        //Inc/Dec 8-bit
        case 0x04: { uint8_t result = ++b; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, (result & 0x0F) == 0x00); cycles += 4; } break;  // INC B
        case 0x0C: { uint8_t result = ++c; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, (result & 0x0F) == 0x00); cycles += 4; } break;  // INC C
        case 0x14: { uint8_t result = ++d; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, (result & 0x0F) == 0x00); cycles += 4; } break;  // INC D
        case 0x1C: { uint8_t result = ++e; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, (result & 0x0F) == 0x00); cycles += 4; } break;  // INC E
        case 0x24: { uint8_t result = ++h; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, (result & 0x0F) == 0x00); cycles += 4; } break;  // INC H
        case 0x2C: { uint8_t result = ++l; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, (result & 0x0F) == 0x00); cycles += 4; } break;  // INC L
        case 0x3C: { uint8_t result = ++a; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, (result & 0x0F) == 0x00); cycles += 4; } break;  // INC A

        case 0x05: { uint8_t result = --b; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, true); setFlag(H_FLAG, (result & 0x0F) == 0x0F); cycles += 4; } break;  // DEC B
        case 0x0D: { uint8_t result = --c; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, true); setFlag(H_FLAG, (result & 0x0F) == 0x0F); cycles += 4; } break;  // DEC C
        case 0x15: { uint8_t result = --d; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, true); setFlag(H_FLAG, (result & 0x0F) == 0x0F); cycles += 4; } break;  // DEC D
        case 0x1D: { uint8_t result = --e; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, true); setFlag(H_FLAG, (result & 0x0F) == 0x0F); cycles += 4; } break;  // DEC E
        case 0x25: { uint8_t result = --h; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, true); setFlag(H_FLAG, (result & 0x0F) == 0x0F); cycles += 4; } break;  // DEC H
        case 0x2D: { uint8_t result = --l; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, true); setFlag(H_FLAG, (result & 0x0F) == 0x0F); cycles += 4; } break;  // DEC L
        case 0x3D: { uint8_t result = --a; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, true); setFlag(H_FLAG, (result & 0x0F) == 0x0F); cycles += 4; } break;  // DEC A

        //Inc/Dec 16 bit
        case 0x03: bc++; cycles += 8; break;  // INC BC
        case 0x13: de++; cycles += 8; break;  // INC DE
        case 0x23: hl++; cycles += 8; break;  // INC HL
        case 0x33: sp++; cycles += 8; break;  // INC SP

        case 0x0B: bc--; cycles += 8; break;  // DEC BC
        case 0x1B: de--; cycles += 8; break;  // DEC DE
        case 0x2B: hl--; cycles += 8; break;  // DEC HL
        case 0x3B: sp--; cycles += 8; break;  // DEC SP

        //Xor
        case 0xA8: a ^= b; setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, false); setFlag(C_FLAG, false); cycles += 4; break;
        case 0xA9: a ^= c; setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, false); setFlag(C_FLAG, false); cycles += 4; break;
        case 0xAA: a ^= d; setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, false); setFlag(C_FLAG, false); cycles += 4; break;
        case 0xAB: a ^= e; setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, false); setFlag(C_FLAG, false); cycles += 4; break;
        case 0xAC: a ^= h; setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, false); setFlag(C_FLAG, false); cycles += 4; break;
        case 0xAD: a ^= l; setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, false); setFlag(C_FLAG, false); cycles += 4; break;
        case 0xAE: a ^= mmu.read(hl); setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, false); setFlag(C_FLAG, false); cycles += 8; break;
        case 0xAF: a = 0; setFlag(Z_FLAG, true); setFlag(N_FLAG, false); setFlag(H_FLAG, false); setFlag(C_FLAG, false); cycles += 4; break;  // XOR A
        case 0xEE: { uint8_t val = mmu.read(pc++); a ^= val; setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, false); setFlag(C_FLAG, false); cycles += 8; } break;  // XOR n

        case 0xB8: { uint8_t result = a - b; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, true); setFlag(H_FLAG, (a & 0x0F) < (b & 0x0F)); setFlag(C_FLAG, a < b); cycles += 4; } break;
        case 0xB9: { uint8_t result = a - c; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, true); setFlag(H_FLAG, (a & 0x0F) < (c & 0x0F)); setFlag(C_FLAG, a < c); cycles += 4; } break;
        case 0xFE: { uint8_t val = mmu.read(pc++); uint8_t result = a - val; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, true); setFlag(H_FLAG, (a & 0x0F) < (val & 0x0F)); setFlag(C_FLAG, a < val); cycles += 8; } break;  // CP n

        case 0x18: { int8_t offset = static_cast<int8_t>(mmu.read(pc++)); pc += offset; cycles += 12; } break;  // JR n
        case 0x20: { int8_t offset = static_cast<int8_t>(mmu.read(pc++)); if (!getFlag(Z_FLAG)) { pc += offset; cycles += 12; } else { cycles += 8; } } break;  // JR NZ, n
        case 0x28: { int8_t offset = static_cast<int8_t>(mmu.read(pc++)); if (getFlag(Z_FLAG)) { pc += offset; cycles += 12; } else { cycles += 8; } } break;  // JR Z, n
        case 0x30: { int8_t offset = static_cast<int8_t>(mmu.read(pc++)); if (!getFlag(C_FLAG)) { pc += offset; cycles += 12; } else { cycles += 8; } } break;  // JR NC, n
        case 0x38: { int8_t offset = static_cast<int8_t>(mmu.read(pc++)); if (getFlag(C_FLAG)) { pc += offset; cycles += 12; } else { cycles += 8; } } break;  // JR C, n

        //
        case 0x22: mmu.write(hl++, a); cycles += 8; break;  // LD (HL+), A
        case 0x2A: a = mmu.read(hl++); cycles += 8; break;  // LD A, (HL+)
        case 0x32: mmu.write(hl--, a); cycles += 8; break;  // LD (HL-), A
        case 0x3A: a = mmu.read(hl--); cycles += 8; break;  // LD A, (HL-)

        // ====================================================================
        // SUB (subtract)
        // ====================================================================
        case 0x90: { uint8_t result = a - b; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, true); setFlag(H_FLAG, (a & 0x0F) < (b & 0x0F)); setFlag(C_FLAG, a < b); a = result; cycles += 4; } break;
        case 0x91: { uint8_t result = a - c; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, true); setFlag(H_FLAG, (a & 0x0F) < (c & 0x0F)); setFlag(C_FLAG, a < c); a = result; cycles += 4; } break;
        case 0x92: { uint8_t result = a - d; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, true); setFlag(H_FLAG, (a & 0x0F) < (d & 0x0F)); setFlag(C_FLAG, a < d); a = result; cycles += 4; } break;
        case 0x93: { uint8_t result = a - e; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, true); setFlag(H_FLAG, (a & 0x0F) < (e & 0x0F)); setFlag(C_FLAG, a < e); a = result; cycles += 4; } break;
        case 0x94: { uint8_t result = a - h; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, true); setFlag(H_FLAG, (a & 0x0F) < (h & 0x0F)); setFlag(C_FLAG, a < h); a = result; cycles += 4; } break;
        case 0x95: { uint8_t result = a - l; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, true); setFlag(H_FLAG, (a & 0x0F) < (l & 0x0F)); setFlag(C_FLAG, a < l); a = result; cycles += 4; } break;
        case 0x96: { uint8_t val = mmu.read(hl); uint8_t result = a - val; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, true); setFlag(H_FLAG, (a & 0x0F) < (val & 0x0F)); setFlag(C_FLAG, a < val); a = result; cycles += 8; } break;
        case 0x97: a = 0; setFlag(Z_FLAG, true); setFlag(N_FLAG, true); setFlag(H_FLAG, false); setFlag(C_FLAG, false); cycles += 4; break;  // SUB A
        case 0xD6: { uint8_t val = mmu.read(pc++); uint8_t result = a - val; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, true); setFlag(H_FLAG, (a & 0x0F) < (val & 0x0F)); setFlag(C_FLAG, a < val); a = result; cycles += 8; } break;  // SUB n

        // ====================================================================
        // AND
        // ====================================================================
        case 0xA0: a &= b; setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, true); setFlag(C_FLAG, false); cycles += 4; break;
        case 0xA1: a &= c; setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, true); setFlag(C_FLAG, false); cycles += 4; break;
        case 0xA2: a &= d; setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, true); setFlag(C_FLAG, false); cycles += 4; break;
        case 0xA3: a &= e; setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, true); setFlag(C_FLAG, false); cycles += 4; break;
        case 0xA4: a &= h; setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, true); setFlag(C_FLAG, false); cycles += 4; break;
        case 0xA5: a &= l; setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, true); setFlag(C_FLAG, false); cycles += 4; break;
        case 0xA6: a &= mmu.read(hl); setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, true); setFlag(C_FLAG, false); cycles += 8; break;
        case 0xA7: setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, true); setFlag(C_FLAG, false); cycles += 4; break;  // AND A
        case 0xE6: { uint8_t val = mmu.read(pc++); a &= val; setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, true); setFlag(C_FLAG, false); cycles += 8; } break;  // AND n

        // ====================================================================
        // OR
        // ====================================================================
        case 0xB0: a |= b; setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, false); setFlag(C_FLAG, false); cycles += 4; break;
        case 0xB1: a |= c; setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, false); setFlag(C_FLAG, false); cycles += 4; break;
        case 0xB2: a |= d; setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, false); setFlag(C_FLAG, false); cycles += 4; break;
        case 0xB3: a |= e; setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, false); setFlag(C_FLAG, false); cycles += 4; break;
        case 0xB4: a |= h; setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, false); setFlag(C_FLAG, false); cycles += 4; break;
        case 0xB5: a |= l; setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, false); setFlag(C_FLAG, false); cycles += 4; break;
        case 0xB6: a |= mmu.read(hl); setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, false); setFlag(C_FLAG, false); cycles += 8; break;
        case 0xB7: setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, false); setFlag(C_FLAG, false); cycles += 4; break;  // OR A
        case 0xF6: { uint8_t val = mmu.read(pc++); a |= val; setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, false); setFlag(C_FLAG, false); cycles += 8; } break;  // OR n

        // ====================================================================
        // CP (plus de variantes)
        // ====================================================================
        case 0xBA: { uint8_t result = a - d; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, true); setFlag(H_FLAG, (a & 0x0F) < (d & 0x0F)); setFlag(C_FLAG, a < d); cycles += 4; } break;
        case 0xBB: { uint8_t result = a - e; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, true); setFlag(H_FLAG, (a & 0x0F) < (e & 0x0F)); setFlag(C_FLAG, a < e); cycles += 4; } break;
        case 0xBC: { uint8_t result = a - h; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, true); setFlag(H_FLAG, (a & 0x0F) < (h & 0x0F)); setFlag(C_FLAG, a < h); cycles += 4; } break;
        case 0xBD: { uint8_t result = a - l; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, true); setFlag(H_FLAG, (a & 0x0F) < (l & 0x0F)); setFlag(C_FLAG, a < l); cycles += 4; } break;
        case 0xBE: { uint8_t val = mmu.read(hl); uint8_t result = a - val; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, true); setFlag(H_FLAG, (a & 0x0F) < (val & 0x0F)); setFlag(C_FLAG, a < val); cycles += 8; } break;
        case 0xBF: setFlag(Z_FLAG, true); setFlag(N_FLAG, true); setFlag(H_FLAG, false); setFlag(C_FLAG, false); cycles += 4; break;  // CP A

        // ====================================================================
        // RST (restart - appels de fonction fixes)
        // ====================================================================
        case 0xC7: mmu.write(--sp, (pc >> 8) & 0xFF); mmu.write(--sp, pc & 0xFF); pc = 0x0000; cycles += 16; break;  // RST 00H
        case 0xCF: mmu.write(--sp, (pc >> 8) & 0xFF); mmu.write(--sp, pc & 0xFF); pc = 0x0008; cycles += 16; break;  // RST 08H
        case 0xD7: mmu.write(--sp, (pc >> 8) & 0xFF); mmu.write(--sp, pc & 0xFF); pc = 0x0010; cycles += 16; break;  // RST 10H
        case 0xDF: mmu.write(--sp, (pc >> 8) & 0xFF); mmu.write(--sp, pc & 0xFF); pc = 0x0018; cycles += 16; break;  // RST 18H
        case 0xE7: mmu.write(--sp, (pc >> 8) & 0xFF); mmu.write(--sp, pc & 0xFF); pc = 0x0020; cycles += 16; break;  // RST 20H
        case 0xEF: mmu.write(--sp, (pc >> 8) & 0xFF); mmu.write(--sp, pc & 0xFF); pc = 0x0028; cycles += 16; break;  // RST 28H
        case 0xF7: mmu.write(--sp, (pc >> 8) & 0xFF); mmu.write(--sp, pc & 0xFF); pc = 0x0030; cycles += 16; break;  // RST 30H
        case 0xFF: mmu.write(--sp, (pc >> 8) & 0xFF); mmu.write(--sp, pc & 0xFF); pc = 0x0038; cycles += 16; break;  // RST 38H

        // ====================================================================
        // RET variants
        // ====================================================================
        case 0xD0: { if (!getFlag(C_FLAG)) { uint8_t lo = mmu.read(sp++); uint8_t hi = mmu.read(sp++); pc = (hi << 8) | lo; cycles += 20; } else { cycles += 8; } } break;  // RET NC
        case 0xD8: { if (getFlag(C_FLAG)) { uint8_t lo = mmu.read(sp++); uint8_t hi = mmu.read(sp++); pc = (hi << 8) | lo; cycles += 20; } else { cycles += 8; } } break;  // RET C
        case 0xD9: { ime = true; uint8_t lo = mmu.read(sp++); uint8_t hi = mmu.read(sp++); pc = (hi << 8) | lo; cycles += 16; } break;  // RETI

        // ====================================================================
        // JP variants (plus complets)
        // ====================================================================
        case 0xD2: { uint16_t addr = mmu.read(pc++) | (mmu.read(pc++) << 8); if (!getFlag(C_FLAG)) { pc = addr; cycles += 16; } else { cycles += 12; } } break;  // JP NC, nn
        case 0xDA: { uint16_t addr = mmu.read(pc++) | (mmu.read(pc++) << 8); if (getFlag(C_FLAG)) { pc = addr; cycles += 16; } else { cycles += 12; } } break;  // JP C, nn

        // ====================================================================
        // CALL variants
        // ====================================================================
        case 0xD4: { uint16_t addr = mmu.read(pc++) | (mmu.read(pc++) << 8); if (!getFlag(C_FLAG)) { mmu.write(--sp, (pc >> 8) & 0xFF); mmu.write(--sp, pc & 0xFF); pc = addr; cycles += 24; } else { cycles += 12; } } break;  // CALL NC, nn
        case 0xDC: { uint16_t addr = mmu.read(pc++) | (mmu.read(pc++) << 8); if (getFlag(C_FLAG)) { mmu.write(--sp, (pc >> 8) & 0xFF); mmu.write(--sp, pc & 0xFF); pc = addr; cycles += 24; } else { cycles += 12; } } break;  // CALL C, nn

        // ====================================================================
        // Rotation (RLCA, RRCA, RLA, RRA)
        // ====================================================================
        case 0x07: { uint8_t carry = (a & 0x80) >> 7; a = (a << 1) | carry; setFlag(Z_FLAG, false); setFlag(N_FLAG, false); setFlag(H_FLAG, false); setFlag(C_FLAG, carry != 0); cycles += 4; } break;  // RLCA
        case 0x0F: { uint8_t carry = a & 0x01; a = (a >> 1) | (carry << 7); setFlag(Z_FLAG, false); setFlag(N_FLAG, false); setFlag(H_FLAG, false); setFlag(C_FLAG, carry != 0); cycles += 4; } break;  // RRCA
        case 0x17: { uint8_t carry = getFlag(C_FLAG) ? 1 : 0; uint8_t newCarry = (a & 0x80) >> 7; a = (a << 1) | carry; setFlag(Z_FLAG, false); setFlag(N_FLAG, false); setFlag(H_FLAG, false); setFlag(C_FLAG, newCarry != 0); cycles += 4; } break;  // RLA
        case 0x1F: { uint8_t carry = getFlag(C_FLAG) ? 1 : 0; uint8_t newCarry = a & 0x01; a = (a >> 1) | (carry << 7); setFlag(Z_FLAG, false); setFlag(N_FLAG, false); setFlag(H_FLAG, false); setFlag(C_FLAG, newCarry != 0); cycles += 4; } break;  // RRA

        // ====================================================================
        // DAA (Decimal Adjust Accumulator)
        // ====================================================================
        case 0x27: {
            uint16_t temp = a;
            if (!getFlag(N_FLAG)) {
                if (getFlag(H_FLAG) || (temp & 0x0F) > 9) temp += 0x06;
                if (getFlag(C_FLAG) || temp > 0x9F) temp += 0x60;
            } else {
                if (getFlag(H_FLAG)) temp = (temp - 6) & 0xFF;
                if (getFlag(C_FLAG)) temp -= 0x60;
            }
            setFlag(H_FLAG, false);
            if (temp > 0xFF) setFlag(C_FLAG, true);
            a = temp & 0xFF;
            setFlag(Z_FLAG, a == 0);
            cycles += 4;
        } break;

        // ====================================================================
        // CPL / SCF / CCF
        // ====================================================================
        case 0x2F: a = ~a; setFlag(N_FLAG, true); setFlag(H_FLAG, true); cycles += 4; break;  // CPL
        case 0x37: setFlag(N_FLAG, false); setFlag(H_FLAG, false); setFlag(C_FLAG, true); cycles += 4; break;  // SCF
        case 0x3F: setFlag(N_FLAG, false); setFlag(H_FLAG, false); setFlag(C_FLAG, !getFlag(C_FLAG)); cycles += 4; break;  // CCF

        // ====================================================================
        // INC/DEC (HL)
        // ====================================================================
        case 0x34: { uint8_t val = mmu.read(hl); uint8_t result = val + 1; mmu.write(hl, result); setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, (result & 0x0F) == 0x00); cycles += 12; } break;  // INC (HL)
        case 0x35: { uint8_t val = mmu.read(hl); uint8_t result = val - 1; mmu.write(hl, result); setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, true); setFlag(H_FLAG, (result & 0x0F) == 0x0F); cycles += 12; } break;  // DEC (HL)

        // ====================================================================
        // ADD HL, rr (16-bit addition)
        // ====================================================================
        case 0x09: { uint32_t result = hl + bc; setFlag(N_FLAG, false); setFlag(H_FLAG, ((hl & 0x0FFF) + (bc & 0x0FFF)) > 0x0FFF); setFlag(C_FLAG, result > 0xFFFF); hl = result & 0xFFFF; cycles += 8; } break;  // ADD HL, BC
        case 0x19: { uint32_t result = hl + de; setFlag(N_FLAG, false); setFlag(H_FLAG, ((hl & 0x0FFF) + (de & 0x0FFF)) > 0x0FFF); setFlag(C_FLAG, result > 0xFFFF); hl = result & 0xFFFF; cycles += 8; } break;  // ADD HL, DE
        case 0x29: { uint32_t result = hl + hl; setFlag(N_FLAG, false); setFlag(H_FLAG, ((hl & 0x0FFF) + (hl & 0x0FFF)) > 0x0FFF); setFlag(C_FLAG, result > 0xFFFF); hl = result & 0xFFFF; cycles += 8; } break;  // ADD HL, HL
        case 0x39: { uint32_t result = hl + sp; setFlag(N_FLAG, false); setFlag(H_FLAG, ((hl & 0x0FFF) + (sp & 0x0FFF)) > 0x0FFF); setFlag(C_FLAG, result > 0xFFFF); hl = result & 0xFFFF; cycles += 8; } break;  // ADD HL, SP

        // ====================================================================
        // ADD SP, n
        // ====================================================================
        case 0xE8: {
            int8_t offset = static_cast<int8_t>(mmu.read(pc++));
            uint16_t result = sp + offset;
            setFlag(Z_FLAG, false);
            setFlag(N_FLAG, false);
            setFlag(H_FLAG, ((sp & 0x0F) + (offset & 0x0F)) > 0x0F);
            setFlag(C_FLAG, ((sp & 0xFF) + (offset & 0xFF)) > 0xFF);
            sp = result;
            cycles += 16;
        } break;
        //
        case 0xC3: pc = mmu.read(pc) | (mmu.read(pc + 1) << 8); cycles += 16; break;  // JP nn
        case 0xC2: { uint16_t addr = mmu.read(pc++) | (mmu.read(pc++) << 8); if (!getFlag(Z_FLAG)) { pc = addr; cycles += 16; } else { cycles += 12; } } break;  // JP NZ, nn
        case 0xCA: { uint16_t addr = mmu.read(pc++) | (mmu.read(pc++) << 8); if (getFlag(Z_FLAG)) { pc = addr; cycles += 16; } else { cycles += 12; } } break;  // JP Z, nn
        case 0xE9: pc = hl; cycles += 4; break;  // JP (HL)

        case 0xCD: { uint16_t addr = mmu.read(pc++) | (mmu.read(pc++) << 8); mmu.write(--sp, (pc >> 8) & 0xFF); mmu.write(--sp, pc & 0xFF); pc = addr; cycles += 24; } break;  // CALL nn
        case 0xC4: { uint16_t addr = mmu.read(pc++) | (mmu.read(pc++) << 8); if (!getFlag(Z_FLAG)) { mmu.write(--sp, (pc >> 8) & 0xFF); mmu.write(--sp, pc & 0xFF); pc = addr; cycles += 24; } else { cycles += 12; } } break;  // CALL NZ, nn
        case 0xCC: { uint16_t addr = mmu.read(pc++) | (mmu.read(pc++) << 8); if (getFlag(Z_FLAG)) { mmu.write(--sp, (pc >> 8) & 0xFF); mmu.write(--sp, pc & 0xFF); pc = addr; cycles += 24; } else { cycles += 12; } } break;  // CALL Z, nn

        case 0xC9: { uint8_t lo = mmu.read(sp++); uint8_t hi = mmu.read(sp++); pc = (hi << 8) | lo; cycles += 16; } break;  // RET
        case 0xC0: { if (!getFlag(Z_FLAG)) { uint8_t lo = mmu.read(sp++); uint8_t hi = mmu.read(sp++); pc = (hi << 8) | lo; cycles += 20; } else { cycles += 8; } } break;  // RET NZ
        case 0xC8: { if (getFlag(Z_FLAG)) { uint8_t lo = mmu.read(sp++); uint8_t hi = mmu.read(sp++); pc = (hi << 8) | lo; cycles += 20; } else { cycles += 8; } } break;  // RET Z

        case 0x76: halted = true; cycles += 4; break;  // HALT
        case 0xF3: ime = false; cycles += 4; break;  // DI
        case 0xFB: ime = true; cycles += 4; break;  // EI

        case 0xCB: {
            uint8_t cb_opcode = mmu.read(pc++);
            executeCB(cb_opcode);
        } break;

        default:
            //LOG_ERROR("Unimplemented opcode: {:#04x} at PC: {:#06x}", opcode, pc - 1);
            cycles += 4;
            break;
    }
}

void GB_CPU::executeCB(uint8_t opcode) {
    uint8_t bit = (opcode >> 3) & 0x07;   // Bits 3-5
    uint8_t reg = opcode & 0x07;          // Bits 0-2

    // Helper pour lire/écrire les registres
    auto getRegister = [&](uint8_t r) -> uint8_t& {
        switch (r) {
            case 0: return b;
            case 1: return c;
            case 2: return d;
            case 3: return e;
            case 4: return h;
            case 5: return l;
            case 7: return a;
            default: {
                static uint8_t dummy = 0;
                return dummy;
            }
        }
    };

    // ====================================================================
    // BIT b, r (test bit)
    // ====================================================================
    if ((opcode & 0xC0) == 0x40) {
        uint8_t val = 0;
        if (reg == 6) {
            val = mmu.read(hl);
            cycles += 4;  // (HL) prend plus de cycles
        } else {
            val = getRegister(reg);
        }

        setFlag(Z_FLAG, (val & (1 << bit)) == 0);
        setFlag(N_FLAG, false);
        setFlag(H_FLAG, true);
        cycles += 8;
        return;
    }

    // ====================================================================
    // RES b, r (reset bit)
    // ====================================================================
    if ((opcode & 0xC0) == 0x80) {
        if (reg == 6) {
            uint8_t val = mmu.read(hl);
            val &= ~(1 << bit);
            mmu.write(hl, val);
            cycles += 16;
        } else {
            getRegister(reg) &= ~(1 << bit);
            cycles += 8;
        }
        return;
    }

    // ====================================================================
    // SET b, r (set bit)
    // ====================================================================
    if ((opcode & 0xC0) == 0xC0) {
        if (reg == 6) {
            uint8_t val = mmu.read(hl);
            val |= (1 << bit);
            mmu.write(hl, val);
            cycles += 16;
        } else {
            getRegister(reg) |= (1 << bit);
            cycles += 8;
        }
        return;
    }

    // ====================================================================
    // ROTATIONS & SHIFTS (0x00-0x3F)
    // ====================================================================

    // Helper pour appliquer sur registre ou (HL)
    auto applyOp = [&](auto operation) {
        if (reg == 6) {
            uint8_t val = mmu.read(hl);
            val = operation(val);
            mmu.write(hl, val);
            cycles += 16;
        } else {
            getRegister(reg) = operation(getRegister(reg));
            cycles += 8;
        }
    };

    switch (opcode & 0xF8) {
        // RLC (rotate left circular)
        case 0x00:
            applyOp([&](uint8_t val) {
                uint8_t carry = (val & 0x80) >> 7;
                val = (val << 1) | carry;
                setFlag(Z_FLAG, val == 0);
                setFlag(N_FLAG, false);
                setFlag(H_FLAG, false);
                setFlag(C_FLAG, carry != 0);
                return val;
            });
            break;

        // RRC (rotate right circular)
        case 0x08:
            applyOp([&](uint8_t val) {
                uint8_t carry = val & 0x01;
                val = (val >> 1) | (carry << 7);
                setFlag(Z_FLAG, val == 0);
                setFlag(N_FLAG, false);
                setFlag(H_FLAG, false);
                setFlag(C_FLAG, carry != 0);
                return val;
            });
            break;

        // RL (rotate left through carry)
        case 0x10:
            applyOp([&](uint8_t val) {
                uint8_t oldCarry = getFlag(C_FLAG) ? 1 : 0;
                uint8_t newCarry = (val & 0x80) >> 7;
                val = (val << 1) | oldCarry;
                setFlag(Z_FLAG, val == 0);
                setFlag(N_FLAG, false);
                setFlag(H_FLAG, false);
                setFlag(C_FLAG, newCarry != 0);
                return val;
            });
            break;

        // RR (rotate right through carry)
        case 0x18:
            applyOp([&](uint8_t val) {
                uint8_t oldCarry = getFlag(C_FLAG) ? 1 : 0;
                uint8_t newCarry = val & 0x01;
                val = (val >> 1) | (oldCarry << 7);
                setFlag(Z_FLAG, val == 0);
                setFlag(N_FLAG, false);
                setFlag(H_FLAG, false);
                setFlag(C_FLAG, newCarry != 0);
                return val;
            });
            break;

        // SLA (shift left arithmetic)
        case 0x20:
            applyOp([&](uint8_t val) {
                uint8_t carry = (val & 0x80) >> 7;
                val = val << 1;
                setFlag(Z_FLAG, val == 0);
                setFlag(N_FLAG, false);
                setFlag(H_FLAG, false);
                setFlag(C_FLAG, carry != 0);
                return val;
            });
            break;

        // SRA (shift right arithmetic - preserve sign bit)
        case 0x28:
            applyOp([&](uint8_t val) {
                uint8_t carry = val & 0x01;
                uint8_t sign = val & 0x80;
                val = (val >> 1) | sign;
                setFlag(Z_FLAG, val == 0);
                setFlag(N_FLAG, false);
                setFlag(H_FLAG, false);
                setFlag(C_FLAG, carry != 0);
                return val;
            });
            break;

        // SWAP (swap nibbles)
        case 0x30:
            applyOp([&](uint8_t val) {
                val = ((val & 0x0F) << 4) | ((val & 0xF0) >> 4);
                setFlag(Z_FLAG, val == 0);
                setFlag(N_FLAG, false);
                setFlag(H_FLAG, false);
                setFlag(C_FLAG, false);
                return val;
            });
            break;

        // SRL (shift right logical)
        case 0x38:
            applyOp([&](uint8_t val) {
                uint8_t carry = val & 0x01;
                val = val >> 1;
                setFlag(Z_FLAG, val == 0);
                setFlag(N_FLAG, false);
                setFlag(H_FLAG, false);
                setFlag(C_FLAG, carry != 0);
                return val;
            });
            break;

        default:
            LOG_ERROR("Unimplemented CB opcode: {:#04x}", opcode);
            cycles += 8;
            break;
    }
}