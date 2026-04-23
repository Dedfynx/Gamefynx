#include "core/gameboy/GB_CPU.h"

#include "GB_PPU.h"
#include "core/gameboy/GB_MMU.h"
#include "core/gameboy/GB_PPU.h"
#include "core/gameboy/GB_Timer.h"
#include "utils/Logger.h"

GB_CPU::GB_CPU(GB_MMU& mmu, GB_PPU& ppu, GB_Timer& timer) : mmu(mmu), ppu(ppu), timer(timer) {
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
    imeScheduled = false;
    halted = false;
    cycles = 0;

    LOG_INFO("GB CPU reset - PC: {:#06x}", pc);
}

void GB_CPU::step() {
    handleInterrupts(mmu);

    if (imeScheduled) {
        ime = true;
        imeScheduled = false;
    }

    if (halted) {
        uint8_t IF = mmu.read(0xFF0F);
        uint8_t IE = mmu.read(0xFFFF);
        if ((IF & IE & 0x1F) != 0) {
            halted = false;
        } else {
            addCycles(4);
            return;
        }
    }

    uint8_t opcode = mmu.read(pc++);
    addCycles(4);
    execute(opcode);

    mmu.dbg_serial();
}

void GB_CPU::execute(uint8_t opcode) {
    switch (opcode) {
        case 0x00: addCycles(4); break; //NOP

    case 0x10:  // STOP
        pc++;
        // On peut ignorer STOP pour les tests
        addCycles(4);
        break;

        case 0x01: c = mmu.read(pc++); b = mmu.read(pc++); addCycles(12); break;  // LD BC, nn
        case 0x11: e = mmu.read(pc++); d = mmu.read(pc++); addCycles(12); break;  // LD DE, nn
        case 0x21: l = mmu.read(pc++); h = mmu.read(pc++); addCycles(12); break;  // LD HL, nn
        case 0x31: sp = mmu.read(pc++) | (mmu.read(pc++) << 8); addCycles(12); break;  // LD SP, nn

        case 0x06: b = mmu.read(pc++); addCycles(8); break;  // LD B, n
        case 0x0E: c = mmu.read(pc++); addCycles(8); break;  // LD C, n
        case 0x16: d = mmu.read(pc++); addCycles(8); break;  // LD D, n
        case 0x1E: e = mmu.read(pc++); addCycles(8); break;  // LD E, n
        case 0x26: h = mmu.read(pc++); addCycles(8); break;  // LD H, n
        case 0x2E: l = mmu.read(pc++); addCycles(8); break;  // LD L, n
        case 0x3E: a = mmu.read(pc++); addCycles(8); break;  // LD A, n

        case 0x40: addCycles(4); break;  // LD B, B
        case 0x41: b = c; addCycles(4); break;  // LD B, C
        case 0x42: b = d; addCycles(4); break;  // LD B, D
        case 0x43: b = e; addCycles(4); break;  // LD B, E
        case 0x44: b = h; addCycles(4); break;  // LD B, H
        case 0x45: b = l; addCycles(4); break;  // LD B, L
        case 0x46: b = mmu.read(hl); addCycles(8); break;  // LD B, (HL)
        case 0x47: b = a; addCycles(4); break;  // LD B, A

        case 0x48: c = b; addCycles(4); break;  // LD C, B
        case 0x49: addCycles(4); break;  // LD C, C
        case 0x4A: c = d; addCycles(4); break;  // LD C, D
        case 0x4B: c = e; addCycles(4); break;  // LD C, E
        case 0x4C: c = h; addCycles(4); break;  // LD C, H
        case 0x4D: c = l; addCycles(4); break;  // LD C, L
        case 0x4E: c = mmu.read(hl); addCycles(8); break;  // LD C, (HL)
        case 0x4F: c = a; addCycles(4); break;  // LD C, A

        case 0x50: d = b; addCycles(4); break;  // LD D, B
        case 0x51: d = c; addCycles(4); break;  // LD D, C
        case 0x52: addCycles(4); break;  // LD D, D
        case 0x53: d = e; addCycles(4); break;  // LD D, E
        case 0x54: d = h; addCycles(4); break;  // LD D, H
        case 0x55: d = l; addCycles(4); break;  // LD D, L
        case 0x56: d = mmu.read(hl); addCycles(8); break;  // LD D, (HL)
        case 0x57: d = a; addCycles(4); break;  // LD D, A

        case 0x58: e = b; addCycles(4); break;  // LD E, B
        case 0x59: e = c; addCycles(4); break;  // LD E, C
        case 0x5A: e = d; addCycles(4); break;  // LD E, D
        case 0x5B: addCycles(4); break;  // LD E, E
        case 0x5C: e = h; addCycles(4); break;  // LD E, H
        case 0x5D: e = l; addCycles(4); break;  // LD E, L
        case 0x5E: e = mmu.read(hl); addCycles(8); break;  // LD E, (HL)
        case 0x5F: e = a; addCycles(4); break;  // LD E, A

        case 0x60: h = b; addCycles(4); break;  // LD H, B
        case 0x61: h = c; addCycles(4); break;  // LD H, C
        case 0x62: h = d; addCycles(4); break;  // LD H, D
        case 0x63: h = e; addCycles(4); break;  // LD H, E
        case 0x64: addCycles(4); break;  // LD H, H
        case 0x65: h = l; addCycles(4); break;  // LD H, L
        case 0x66: h = mmu.read(hl); addCycles(8); break;  // LD H, (HL)
        case 0x67: h = a; addCycles(4); break;  // LD H, A

        case 0x68: l = b; addCycles(4); break;  // LD L, B
        case 0x69: l = c; addCycles(4); break;  // LD L, C
        case 0x6A: l = d; addCycles(4); break;  // LD L, D
        case 0x6B: l = e; addCycles(4); break;  // LD L, E
        case 0x6C: l = h; addCycles(4); break;  // LD L, H
        case 0x6D: addCycles(4); break;  // LD L, L
        case 0x6E: l = mmu.read(hl); addCycles(8); break;  // LD L, (HL)
        case 0x6F: l = a; addCycles(4); break;  // LD L, A

        case 0x70: mmu.write(hl, b); addCycles(8); break;  // LD (HL), B
        case 0x71: mmu.write(hl, c); addCycles(8); break;  // LD (HL), C
        case 0x72: mmu.write(hl, d); addCycles(8); break;  // LD (HL), D
        case 0x73: mmu.write(hl, e); addCycles(8); break;  // LD (HL), E
        case 0x74: mmu.write(hl, h); addCycles(8); break;  // LD (HL), H
        case 0x75: mmu.write(hl, l); addCycles(8); break;  // LD (HL), L
        case 0x36: mmu.write(hl, mmu.read(pc++)); addCycles(12); break;  // LD (HL), n
        case 0x77: mmu.write(hl, a); addCycles(8); break;  // LD (HL), A

        case 0x78: a = b; addCycles(4); break;  // LD A, B
        case 0x79: a = c; addCycles(4); break;  // LD A, C
        case 0x7A: a = d; addCycles(4); break;  // LD A, D
        case 0x7B: a = e; addCycles(4); break;  // LD A, E
        case 0x7C: a = h; addCycles(4); break;  // LD A, H
        case 0x7D: a = l; addCycles(4); break;  // LD A, L
        case 0x7E: a = mmu.read(hl); addCycles(8); break;  // LD A, (HL)
        case 0x7F: addCycles(4); break;  // LD A, A

        case 0x0A: a = mmu.read(bc); addCycles(8); break;  // LD A, (BC)
        case 0x1A: a = mmu.read(de); addCycles(8); break;  // LD A, (DE)
        case 0xFA: { uint16_t addr = mmu.read(pc++) | (mmu.read(pc++) << 8); a = mmu.read(addr); addCycles(16); } break;  // LD A, (nn)

        case 0x02: mmu.write(bc, a); addCycles(8); break;  // LD (BC), A
        case 0x12: mmu.write(de, a); addCycles(8); break;  // LD (DE), A
        case 0xEA: { uint16_t addr = mmu.read(pc++) | (mmu.read(pc++) << 8); mmu.write(addr, a); addCycles(16); } break;  // LD (nn), A

        case 0xE0: mmu.write(0xFF00 + mmu.read(pc++), a); addCycles(12); break;  // LDH (n), A
        case 0xF0: a = mmu.read(0xFF00 + mmu.read(pc++)); addCycles(12); break;  // LDH A, (n)
        case 0xE2: mmu.write(0xFF00 + c, a); addCycles(8); break;  // LD (C), A
        case 0xF2: a = mmu.read(0xFF00 + c); addCycles(8); break;  // LD A, (C)

        case 0xF8: {
            int8_t offset = static_cast<int8_t>(mmu.read(pc++));
            uint16_t result = sp + offset;
            hl = result;
            setFlag(Z_FLAG, false);
            setFlag(N_FLAG, false);
            setFlag(H_FLAG, ((sp & 0x0F) + (offset & 0x0F)) > 0x0F);
            setFlag(C_FLAG, ((sp & 0xFF) + (offset & 0xFF)) > 0xFF);
            addCycles(12);
        } break;

    case 0x08:  // LD (nn), SP
        {
            uint8_t low = mmu.read(pc++);
            uint8_t high = mmu.read(pc++);
            uint16_t addr = (high << 8) | low;

            mmu.write(addr, sp & 0xFF);         // Low byte
            mmu.write(addr + 1, (sp >> 8) & 0xFF);  // High byte

            addCycles(20);
        }
        break;

        case 0xF9: sp = hl; addCycles(8); break;

        case 0xC5: mmu.write(--sp, b); mmu.write(--sp, c); addCycles(16); break;  // PUSH BC
        case 0xD5: mmu.write(--sp, d); mmu.write(--sp, e); addCycles(16); break;  // PUSH DE
        case 0xE5: mmu.write(--sp, h); mmu.write(--sp, l); addCycles(16); break;  // PUSH HL
        case 0xF5: mmu.write(--sp, a); mmu.write(--sp, f & 0xF0); addCycles(16); break;  // PUSH AF

        case 0xC1: c = mmu.read(sp++); b = mmu.read(sp++); addCycles(12); break;  // POP BC
        case 0xD1: e = mmu.read(sp++); d = mmu.read(sp++); addCycles(12); break;  // POP DE
        case 0xE1: l = mmu.read(sp++); h = mmu.read(sp++); addCycles(12); break;  // POP HL
        case 0xF1: f = mmu.read(sp++) & 0xF0; a = mmu.read(sp++); addCycles(12); break;  // POP AF

        // Add
        case 0x80: { uint8_t val = b; uint16_t result = a + val; setFlag(Z_FLAG, (result & 0xFF) == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, ((a & 0x0F) + (val & 0x0F)) > 0x0F); setFlag(C_FLAG, result > 0xFF); a = result & 0xFF; addCycles(4); } break;
        case 0x81: { uint8_t val = c; uint16_t result = a + val; setFlag(Z_FLAG, (result & 0xFF) == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, ((a & 0x0F) + (val & 0x0F)) > 0x0F); setFlag(C_FLAG, result > 0xFF); a = result & 0xFF; addCycles(4); } break;
        case 0x82: { uint8_t val = d; uint16_t result = a + val; setFlag(Z_FLAG, (result & 0xFF) == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, ((a & 0x0F) + (val & 0x0F)) > 0x0F); setFlag(C_FLAG, result > 0xFF); a = result & 0xFF; addCycles(4); } break;
        case 0x83: { uint8_t val = e; uint16_t result = a + val; setFlag(Z_FLAG, (result & 0xFF) == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, ((a & 0x0F) + (val & 0x0F)) > 0x0F); setFlag(C_FLAG, result > 0xFF); a = result & 0xFF; addCycles(4); } break;
        case 0x84: { uint8_t val = h; uint16_t result = a + val; setFlag(Z_FLAG, (result & 0xFF) == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, ((a & 0x0F) + (val & 0x0F)) > 0x0F); setFlag(C_FLAG, result > 0xFF); a = result & 0xFF; addCycles(4); } break;
        case 0x85: { uint8_t val = l; uint16_t result = a + val; setFlag(Z_FLAG, (result & 0xFF) == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, ((a & 0x0F) + (val & 0x0F)) > 0x0F); setFlag(C_FLAG, result > 0xFF); a = result & 0xFF; addCycles(4); } break;
        case 0x86: { uint8_t val = mmu.read(hl); uint16_t result = a + val; setFlag(Z_FLAG, (result & 0xFF) == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, ((a & 0x0F) + (val & 0x0F)) > 0x0F); setFlag(C_FLAG, result > 0xFF); a = result & 0xFF; addCycles(8); } break;
        case 0x87: { uint16_t result = a + a; setFlag(Z_FLAG, (result & 0xFF) == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, ((a & 0x0F) + (a & 0x0F)) > 0x0F); setFlag(C_FLAG, result > 0xFF); a = result & 0xFF; addCycles(4); } break;
        case 0xC6: { uint8_t val = mmu.read(pc++); uint16_t result = a + val; setFlag(Z_FLAG, (result & 0xFF) == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, ((a & 0x0F) + (val & 0x0F)) > 0x0F); setFlag(C_FLAG, result > 0xFF); a = result & 0xFF; addCycles(8); } break;

        //Inc/Dec 8-bit
        case 0x04: { uint8_t result = ++b; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, (result & 0x0F) == 0x00); addCycles(4); } break;  // INC B
        case 0x0C: { uint8_t result = ++c; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, (result & 0x0F) == 0x00); addCycles(4); } break;  // INC C
        case 0x14: { uint8_t result = ++d; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, (result & 0x0F) == 0x00); addCycles(4); } break;  // INC D
        case 0x1C: { uint8_t result = ++e; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, (result & 0x0F) == 0x00); addCycles(4); } break;  // INC E
        case 0x24: { uint8_t result = ++h; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, (result & 0x0F) == 0x00); addCycles(4); } break;  // INC H
        case 0x2C: { uint8_t result = ++l; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, (result & 0x0F) == 0x00); addCycles(4); } break;  // INC L
        case 0x3C: { uint8_t result = ++a; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, (result & 0x0F) == 0x00); addCycles(4); } break;  // INC A

        case 0x05: { uint8_t result = --b; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, true); setFlag(H_FLAG, (result & 0x0F) == 0x0F); addCycles(4); } break;  // DEC B
        case 0x0D: { uint8_t result = --c; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, true); setFlag(H_FLAG, (result & 0x0F) == 0x0F); addCycles(4); } break;  // DEC C
        case 0x15: { uint8_t result = --d; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, true); setFlag(H_FLAG, (result & 0x0F) == 0x0F); addCycles(4); } break;  // DEC D
        case 0x1D: { uint8_t result = --e; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, true); setFlag(H_FLAG, (result & 0x0F) == 0x0F); addCycles(4); } break;  // DEC E
        case 0x25: { uint8_t result = --h; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, true); setFlag(H_FLAG, (result & 0x0F) == 0x0F); addCycles(4); } break;  // DEC H
        case 0x2D: { uint8_t result = --l; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, true); setFlag(H_FLAG, (result & 0x0F) == 0x0F); addCycles(4); } break;  // DEC L
        case 0x3D: { uint8_t result = --a; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, true); setFlag(H_FLAG, (result & 0x0F) == 0x0F); addCycles(4); } break;  // DEC A

        //Inc/Dec 16 bit
        case 0x03: bc++; addCycles(8); break;  // INC BC
        case 0x13: de++; addCycles(8); break;  // INC DE
        case 0x23: hl++; addCycles(8); break;  // INC HL
        case 0x33: sp++; addCycles(8); break;  // INC SP

        case 0x0B: bc--; addCycles(8); break;  // DEC BC
        case 0x1B: de--; addCycles(8); break;  // DEC DE
        case 0x2B: hl--; addCycles(8); break;  // DEC HL
        case 0x3B: sp--; addCycles(8); break;  // DEC SP

        //add with carry
    case 0xCE:  // ADC A, n
        {
            uint8_t val = mmu.read(pc++);
            int carry = getFlag(C_FLAG) ? 1 : 0;
            int result = a + val + carry;

            setFlag(Z_FLAG, (result & 0xFF) == 0);
            setFlag(N_FLAG, false);
            setFlag(H_FLAG, ((a & 0x0F) + (val & 0x0F) + carry) > 0x0F);
            setFlag(C_FLAG, result > 0xFF);

            a = result & 0xFF;
            addCycles(8);
        }
        break;
    case 0x88: case 0x89: case 0x8A: case 0x8B:
    case 0x8C: case 0x8D: case 0x8E: case 0x8F:
        {
            uint8_t val = 0;
            switch (opcode & 0x07) {
            case 0: val = b; break;
            case 1: val = c; break;
            case 2: val = d; break;
            case 3: val = e; break;
            case 4: val = h; break;
            case 5: val = l; break;
            case 6: val = mmu.read(hl); addCycles(4); break;
            case 7: val = a; break;
            }

            int carry = getFlag(C_FLAG) ? 1 : 0;
            int result = a + val + carry;

            setFlag(Z_FLAG, (result & 0xFF) == 0);
            setFlag(N_FLAG, false);
            setFlag(H_FLAG, ((a & 0x0F) + (val & 0x0F) + carry) > 0x0F);
            setFlag(C_FLAG, result > 0xFF);

            a = result & 0xFF;
            addCycles(4);
        }
        break;

        //sub with carry
    case 0xDE:  // SBC A, n
        {
            uint8_t val = mmu.read(pc++);
            int carry = getFlag(C_FLAG) ? 1 : 0;
            int result = a - val - carry;

            setFlag(Z_FLAG, (result & 0xFF) == 0);
            setFlag(N_FLAG, true);
            setFlag(H_FLAG, ((a & 0x0F) - (val & 0x0F) - carry) < 0);
            setFlag(C_FLAG, result < 0);

            a = result & 0xFF;
            addCycles(8);
        }
        break;
        // SBC A, r (Subtract with Carry)
    case 0x98: case 0x99: case 0x9A: case 0x9B:
    case 0x9C: case 0x9D: case 0x9E: case 0x9F:
        {
            uint8_t val = 0;
            switch (opcode & 0x07) {
            case 0: val = b; break;
            case 1: val = c; break;
            case 2: val = d; break;
            case 3: val = e; break;
            case 4: val = h; break;
            case 5: val = l; break;
            case 6: val = mmu.read(hl); addCycles(4); break;
            case 7: val = a; break;
            }

            int carry = getFlag(C_FLAG) ? 1 : 0;
            int result = a - val - carry;

            setFlag(Z_FLAG, (result & 0xFF) == 0);
            setFlag(N_FLAG, true);
            setFlag(H_FLAG, ((a & 0x0F) - (val & 0x0F) - carry) < 0);
            setFlag(C_FLAG, result < 0);

            a = result & 0xFF;
            addCycles(4);
        }
        break;
        //Xor
        case 0xA8: a ^= b; setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, false); setFlag(C_FLAG, false); addCycles(4); break;
        case 0xA9: a ^= c; setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, false); setFlag(C_FLAG, false); addCycles(4); break;
        case 0xAA: a ^= d; setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, false); setFlag(C_FLAG, false); addCycles(4); break;
        case 0xAB: a ^= e; setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, false); setFlag(C_FLAG, false); addCycles(4); break;
        case 0xAC: a ^= h; setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, false); setFlag(C_FLAG, false); addCycles(4); break;
        case 0xAD: a ^= l; setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, false); setFlag(C_FLAG, false); addCycles(4); break;
        case 0xAE: a ^= mmu.read(hl); setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, false); setFlag(C_FLAG, false); addCycles(8); break;
        case 0xAF: a = 0; setFlag(Z_FLAG, true); setFlag(N_FLAG, false); setFlag(H_FLAG, false); setFlag(C_FLAG, false); addCycles(4); break;  // XOR A
        case 0xEE: { uint8_t val = mmu.read(pc++); a ^= val; setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, false); setFlag(C_FLAG, false); addCycles(8); } break;  // XOR n

        case 0xB8: { uint8_t result = a - b; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, true); setFlag(H_FLAG, (a & 0x0F) < (b & 0x0F)); setFlag(C_FLAG, a < b); addCycles(4); } break;
        case 0xB9: { uint8_t result = a - c; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, true); setFlag(H_FLAG, (a & 0x0F) < (c & 0x0F)); setFlag(C_FLAG, a < c); addCycles(4); } break;
        case 0xFE: { uint8_t val = mmu.read(pc++); uint8_t result = a - val; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, true); setFlag(H_FLAG, (a & 0x0F) < (val & 0x0F)); setFlag(C_FLAG, a < val); addCycles(8); } break;  // CP n

        case 0x18: { int8_t offset = static_cast<int8_t>(mmu.read(pc++)); pc += offset; addCycles(12); } break;  // JR n
        case 0x20: { int8_t offset = static_cast<int8_t>(mmu.read(pc++)); if (!getFlag(Z_FLAG)) { pc += offset; addCycles(12); } else { addCycles(8); } } break;  // JR NZ, n
        case 0x28: { int8_t offset = static_cast<int8_t>(mmu.read(pc++)); if (getFlag(Z_FLAG)) { pc += offset; addCycles(12); } else { addCycles(8); } } break;  // JR Z, n
        case 0x30: { int8_t offset = static_cast<int8_t>(mmu.read(pc++)); if (!getFlag(C_FLAG)) { pc += offset; addCycles(12); } else { addCycles(8); } } break;  // JR NC, n
        case 0x38: { int8_t offset = static_cast<int8_t>(mmu.read(pc++)); if (getFlag(C_FLAG)) { pc += offset; addCycles(12); } else { addCycles(8); } } break;  // JR C, n

        //
        case 0x22: mmu.write(hl++, a); addCycles(8); break;  // LD (HL+), A
        case 0x2A: a = mmu.read(hl++); addCycles(8); break;  // LD A, (HL+)
        case 0x32: mmu.write(hl--, a); addCycles(8); break;  // LD (HL-), A
        case 0x3A: a = mmu.read(hl--); addCycles(8); break;  // LD A, (HL-)

        // ====================================================================
        // SUB (subtract)
        // ====================================================================
        case 0x90: { uint8_t result = a - b; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, true); setFlag(H_FLAG, (a & 0x0F) < (b & 0x0F)); setFlag(C_FLAG, a < b); a = result; addCycles(4); } break;
        case 0x91: { uint8_t result = a - c; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, true); setFlag(H_FLAG, (a & 0x0F) < (c & 0x0F)); setFlag(C_FLAG, a < c); a = result; addCycles(4); } break;
        case 0x92: { uint8_t result = a - d; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, true); setFlag(H_FLAG, (a & 0x0F) < (d & 0x0F)); setFlag(C_FLAG, a < d); a = result; addCycles(4); } break;
        case 0x93: { uint8_t result = a - e; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, true); setFlag(H_FLAG, (a & 0x0F) < (e & 0x0F)); setFlag(C_FLAG, a < e); a = result; addCycles(4); } break;
        case 0x94: { uint8_t result = a - h; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, true); setFlag(H_FLAG, (a & 0x0F) < (h & 0x0F)); setFlag(C_FLAG, a < h); a = result; addCycles(4); } break;
        case 0x95: { uint8_t result = a - l; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, true); setFlag(H_FLAG, (a & 0x0F) < (l & 0x0F)); setFlag(C_FLAG, a < l); a = result; addCycles(4); } break;
        case 0x96: { uint8_t val = mmu.read(hl); uint8_t result = a - val; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, true); setFlag(H_FLAG, (a & 0x0F) < (val & 0x0F)); setFlag(C_FLAG, a < val); a = result; addCycles(8); } break;
        case 0x97: a = 0; setFlag(Z_FLAG, true); setFlag(N_FLAG, true); setFlag(H_FLAG, false); setFlag(C_FLAG, false); addCycles(4); break;  // SUB A
        case 0xD6: { uint8_t val = mmu.read(pc++); uint8_t result = a - val; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, true); setFlag(H_FLAG, (a & 0x0F) < (val & 0x0F)); setFlag(C_FLAG, a < val); a = result; addCycles(8); } break;  // SUB n

        // ====================================================================
        // AND
        // ====================================================================
        case 0xA0: a &= b; setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, true); setFlag(C_FLAG, false); addCycles(4); break;
        case 0xA1: a &= c; setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, true); setFlag(C_FLAG, false); addCycles(4); break;
        case 0xA2: a &= d; setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, true); setFlag(C_FLAG, false); addCycles(4); break;
        case 0xA3: a &= e; setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, true); setFlag(C_FLAG, false); addCycles(4); break;
        case 0xA4: a &= h; setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, true); setFlag(C_FLAG, false); addCycles(4); break;
        case 0xA5: a &= l; setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, true); setFlag(C_FLAG, false); addCycles(4); break;
        case 0xA6: a &= mmu.read(hl); setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, true); setFlag(C_FLAG, false); addCycles(8); break;
        case 0xA7: setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, true); setFlag(C_FLAG, false); addCycles(4); break;  // AND A
        case 0xE6: { uint8_t val = mmu.read(pc++); a &= val; setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, true); setFlag(C_FLAG, false); addCycles(8); } break;  // AND n

        // ====================================================================
        // OR
        // ====================================================================
        case 0xB0: a |= b; setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, false); setFlag(C_FLAG, false); addCycles(4); break;
        case 0xB1: a |= c; setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, false); setFlag(C_FLAG, false); addCycles(4); break;
        case 0xB2: a |= d; setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, false); setFlag(C_FLAG, false); addCycles(4); break;
        case 0xB3: a |= e; setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, false); setFlag(C_FLAG, false); addCycles(4); break;
        case 0xB4: a |= h; setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, false); setFlag(C_FLAG, false); addCycles(4); break;
        case 0xB5: a |= l; setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, false); setFlag(C_FLAG, false); addCycles(4); break;
        case 0xB6: a |= mmu.read(hl); setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, false); setFlag(C_FLAG, false); addCycles(8); break;
        case 0xB7: setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, false); setFlag(C_FLAG, false); addCycles(4); break;  // OR A
        case 0xF6: { uint8_t val = mmu.read(pc++); a |= val; setFlag(Z_FLAG, a == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, false); setFlag(C_FLAG, false); addCycles(8); } break;  // OR n

        // ====================================================================
        // CP (plus de variantes)
        // ====================================================================
        case 0xBA: { uint8_t result = a - d; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, true); setFlag(H_FLAG, (a & 0x0F) < (d & 0x0F)); setFlag(C_FLAG, a < d); addCycles(4); } break;
        case 0xBB: { uint8_t result = a - e; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, true); setFlag(H_FLAG, (a & 0x0F) < (e & 0x0F)); setFlag(C_FLAG, a < e); addCycles(4); } break;
        case 0xBC: { uint8_t result = a - h; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, true); setFlag(H_FLAG, (a & 0x0F) < (h & 0x0F)); setFlag(C_FLAG, a < h); addCycles(4); } break;
        case 0xBD: { uint8_t result = a - l; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, true); setFlag(H_FLAG, (a & 0x0F) < (l & 0x0F)); setFlag(C_FLAG, a < l); addCycles(4); } break;
        case 0xBE: { uint8_t val = mmu.read(hl); uint8_t result = a - val; setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, true); setFlag(H_FLAG, (a & 0x0F) < (val & 0x0F)); setFlag(C_FLAG, a < val); addCycles(8); } break;
        case 0xBF: setFlag(Z_FLAG, true); setFlag(N_FLAG, true); setFlag(H_FLAG, false); setFlag(C_FLAG, false); addCycles(4); break;  // CP A

        // ====================================================================
        // RST (restart - appels de fonction fixes)
        // ====================================================================
        case 0xC7: mmu.write(--sp, (pc >> 8) & 0xFF); mmu.write(--sp, pc & 0xFF); pc = 0x0000; addCycles(16); break;  // RST 00H
        case 0xCF: mmu.write(--sp, (pc >> 8) & 0xFF); mmu.write(--sp, pc & 0xFF); pc = 0x0008; addCycles(16); break;  // RST 08H
        case 0xD7: mmu.write(--sp, (pc >> 8) & 0xFF); mmu.write(--sp, pc & 0xFF); pc = 0x0010; addCycles(16); break;  // RST 10H
        case 0xDF: mmu.write(--sp, (pc >> 8) & 0xFF); mmu.write(--sp, pc & 0xFF); pc = 0x0018; addCycles(16); break;  // RST 18H
        case 0xE7: mmu.write(--sp, (pc >> 8) & 0xFF); mmu.write(--sp, pc & 0xFF); pc = 0x0020; addCycles(16); break;  // RST 20H
        case 0xEF: mmu.write(--sp, (pc >> 8) & 0xFF); mmu.write(--sp, pc & 0xFF); pc = 0x0028; addCycles(16); break;  // RST 28H
        case 0xF7: mmu.write(--sp, (pc >> 8) & 0xFF); mmu.write(--sp, pc & 0xFF); pc = 0x0030; addCycles(16); break;  // RST 30H
        case 0xFF: mmu.write(--sp, (pc >> 8) & 0xFF); mmu.write(--sp, pc & 0xFF); pc = 0x0038; addCycles(16); break;  // RST 38H

        // ====================================================================
        // RET variants
        // ====================================================================
        case 0xD0: { if (!getFlag(C_FLAG)) { uint8_t lo = mmu.read(sp++); uint8_t hi = mmu.read(sp++); pc = (hi << 8) | lo; addCycles(20); } else { addCycles(8); } } break;  // RET NC
        case 0xD8: { if (getFlag(C_FLAG)) { uint8_t lo = mmu.read(sp++); uint8_t hi = mmu.read(sp++); pc = (hi << 8) | lo; addCycles(20); } else { addCycles(8); } } break;  // RET C
        case 0xD9:  // RETI
            {
                uint8_t lo = mmu.read(sp++);
                uint8_t hi = mmu.read(sp++);
                pc = (hi << 8) | lo;
                ime = true;
                //imeScheduled = false;
                addCycles(16);
            }
            break;

        // ====================================================================
        // JP variants (plus complets)
        // ====================================================================
        case 0xD2: { uint16_t addr = mmu.read(pc++) | (mmu.read(pc++) << 8); if (!getFlag(C_FLAG)) { pc = addr; addCycles(16); } else { addCycles(12); } } break;  // JP NC, nn
        case 0xDA: { uint16_t addr = mmu.read(pc++) | (mmu.read(pc++) << 8); if (getFlag(C_FLAG)) { pc = addr; addCycles(16); } else { addCycles(12); } } break;  // JP C, nn

        // ====================================================================
        // CALL variants
        // ====================================================================
        case 0xD4: { uint16_t addr = mmu.read(pc++) | (mmu.read(pc++) << 8); if (!getFlag(C_FLAG)) { mmu.write(--sp, (pc >> 8) & 0xFF); mmu.write(--sp, pc & 0xFF); pc = addr; addCycles(24); } else { addCycles(12); } } break;  // CALL NC, nn
        case 0xDC: { uint16_t addr = mmu.read(pc++) | (mmu.read(pc++) << 8); if (getFlag(C_FLAG)) { mmu.write(--sp, (pc >> 8) & 0xFF); mmu.write(--sp, pc & 0xFF); pc = addr; addCycles(24); } else { addCycles(12); } } break;  // CALL C, nn

        // ====================================================================
        // Rotation (RLCA, RRCA, RLA, RRA)
        // ====================================================================
        case 0x07: { uint8_t carry = (a & 0x80) >> 7; a = (a << 1) | carry; setFlag(Z_FLAG, false); setFlag(N_FLAG, false); setFlag(H_FLAG, false); setFlag(C_FLAG, carry != 0); addCycles(4); } break;  // RLCA
        case 0x0F: { uint8_t carry = a & 0x01; a = (a >> 1) | (carry << 7); setFlag(Z_FLAG, false); setFlag(N_FLAG, false); setFlag(H_FLAG, false); setFlag(C_FLAG, carry != 0); addCycles(4); } break;  // RRCA
        case 0x17: { uint8_t carry = getFlag(C_FLAG) ? 1 : 0; uint8_t newCarry = (a & 0x80) >> 7; a = (a << 1) | carry; setFlag(Z_FLAG, false); setFlag(N_FLAG, false); setFlag(H_FLAG, false); setFlag(C_FLAG, newCarry != 0); addCycles(4); } break;  // RLA
        case 0x1F: { uint8_t carry = getFlag(C_FLAG) ? 1 : 0; uint8_t newCarry = a & 0x01; a = (a >> 1) | (carry << 7); setFlag(Z_FLAG, false); setFlag(N_FLAG, false); setFlag(H_FLAG, false); setFlag(C_FLAG, newCarry != 0); addCycles(4); } break;  // RRA

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
            addCycles(4);
        } break;

        // ====================================================================
        // CPL / SCF / CCF
        // ====================================================================
        case 0x2F: a = ~a; setFlag(N_FLAG, true); setFlag(H_FLAG, true); addCycles(4); break;  // CPL
        case 0x37: setFlag(N_FLAG, false); setFlag(H_FLAG, false); setFlag(C_FLAG, true); addCycles(4); break;  // SCF
        case 0x3F: setFlag(N_FLAG, false); setFlag(H_FLAG, false); setFlag(C_FLAG, !getFlag(C_FLAG)); addCycles(4); break;  // CCF

        // ====================================================================
        // INC/DEC (HL)
        // ====================================================================
        case 0x34: { uint8_t val = mmu.read(hl); uint8_t result = val + 1; mmu.write(hl, result); setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, false); setFlag(H_FLAG, (result & 0x0F) == 0x00); addCycles(12); } break;  // INC (HL)
        case 0x35: { uint8_t val = mmu.read(hl); uint8_t result = val - 1; mmu.write(hl, result); setFlag(Z_FLAG, result == 0); setFlag(N_FLAG, true); setFlag(H_FLAG, (result & 0x0F) == 0x0F); addCycles(12); } break;  // DEC (HL)

        // ====================================================================
        // ADD HL, rr (16-bit addition)
        // ====================================================================
        case 0x09: { uint32_t result = hl + bc; setFlag(N_FLAG, false); setFlag(H_FLAG, ((hl & 0x0FFF) + (bc & 0x0FFF)) > 0x0FFF); setFlag(C_FLAG, result > 0xFFFF); hl = result & 0xFFFF; addCycles(8); } break;  // ADD HL, BC
        case 0x19: { uint32_t result = hl + de; setFlag(N_FLAG, false); setFlag(H_FLAG, ((hl & 0x0FFF) + (de & 0x0FFF)) > 0x0FFF); setFlag(C_FLAG, result > 0xFFFF); hl = result & 0xFFFF; addCycles(8); } break;  // ADD HL, DE
        case 0x29: { uint32_t result = hl + hl; setFlag(N_FLAG, false); setFlag(H_FLAG, ((hl & 0x0FFF) + (hl & 0x0FFF)) > 0x0FFF); setFlag(C_FLAG, result > 0xFFFF); hl = result & 0xFFFF; addCycles(8); } break;  // ADD HL, HL
        case 0x39: { uint32_t result = hl + sp; setFlag(N_FLAG, false); setFlag(H_FLAG, ((hl & 0x0FFF) + (sp & 0x0FFF)) > 0x0FFF); setFlag(C_FLAG, result > 0xFFFF); hl = result & 0xFFFF; addCycles(8); } break;  // ADD HL, SP

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
            addCycles(16);
        } break;
        //
        case 0xC3: pc = mmu.read(pc) | (mmu.read(pc + 1) << 8); addCycles(16); break;  // JP nn
        case 0xC2: { uint16_t addr = mmu.read(pc++) | (mmu.read(pc++) << 8); if (!getFlag(Z_FLAG)) { pc = addr; addCycles(16); } else { addCycles(12); } } break;  // JP NZ, nn
        case 0xCA: { uint16_t addr = mmu.read(pc++) | (mmu.read(pc++) << 8); if (getFlag(Z_FLAG)) { pc = addr; addCycles(16); } else { addCycles(12); } } break;  // JP Z, nn
        case 0xE9: pc = hl; addCycles(4); break;  // JP (HL)

        case 0xCD: { uint16_t addr = mmu.read(pc++) | (mmu.read(pc++) << 8); mmu.write(--sp, (pc >> 8) & 0xFF); mmu.write(--sp, pc & 0xFF); pc = addr; addCycles(24); } break;  // CALL nn
        case 0xC4: { uint16_t addr = mmu.read(pc++) | (mmu.read(pc++) << 8); if (!getFlag(Z_FLAG)) { mmu.write(--sp, (pc >> 8) & 0xFF); mmu.write(--sp, pc & 0xFF); pc = addr; addCycles(24); } else { addCycles(12); } } break;  // CALL NZ, nn
        case 0xCC: { uint16_t addr = mmu.read(pc++) | (mmu.read(pc++) << 8); if (getFlag(Z_FLAG)) { mmu.write(--sp, (pc >> 8) & 0xFF); mmu.write(--sp, pc & 0xFF); pc = addr; addCycles(24); } else { addCycles(12); } } break;  // CALL Z, nn

        case 0xC9: { uint8_t lo = mmu.read(sp++); uint8_t hi = mmu.read(sp++); pc = (hi << 8) | lo; addCycles(16); } break;  // RET
        case 0xC0: { if (!getFlag(Z_FLAG)) { uint8_t lo = mmu.read(sp++); uint8_t hi = mmu.read(sp++); pc = (hi << 8) | lo; addCycles(20); } else { addCycles(8); } } break;  // RET NZ
        case 0xC8: { if (getFlag(Z_FLAG)) { uint8_t lo = mmu.read(sp++); uint8_t hi = mmu.read(sp++); pc = (hi << 8) | lo; addCycles(20); } else { addCycles(8); } } break;  // RET Z

        case 0x76: // HALT
            if (ime == false && (mmu.read(0xFF0F) & mmu.read(0xFFFF) & 0x1F) != 0) {
                // LE BUG : Le CPU ne s'arrête pas, mais le PC ne s'incrémente pas
                // lors de la lecture de l'opcode suivant (on lit deux fois le même)
                haltBugTriggered = true;
            } else {
                halted = true;
            }
            break;
        case 0xF3: ime = false; imeScheduled = false;addCycles(4); break;  // DI
        case 0xFB: imeScheduled = true; addCycles(4); break;  // EI

        case 0xCB: {
            uint8_t cb_opcode = mmu.read(pc++);
            executeCB(cb_opcode);
        } break;

        default:
            LOG_ERROR("Unimplemented opcode: {:#04x} at PC: {:#06x}", opcode, pc - 1);
            addCycles(4);
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
            addCycles(4);  // (HL) prend plus de cycles
        } else {
            val = getRegister(reg);
        }

        setFlag(Z_FLAG, (val & (1 << bit)) == 0);
        setFlag(N_FLAG, false);
        setFlag(H_FLAG, true);
        addCycles(8);
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
            addCycles(16);
        } else {
            getRegister(reg) &= ~(1 << bit);
            addCycles(8);
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
            addCycles(16);
        } else {
            getRegister(reg) |= (1 << bit);
            addCycles(8);
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
            addCycles(16);
        } else {
            getRegister(reg) = operation(getRegister(reg));
            addCycles(8);
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
            addCycles(8);
            break;
    }
}

void GB_CPU::addCycles(int c)
{
    cycles += c;
    timer.step(c);
    ppu.step(c);
}

void GB_CPU::handleInterrupts(GB_MMU& mmu) {
    uint8_t IF = mmu.read(0xFF0F);
    uint8_t IE = mmu.read(0xFFFF);
    uint8_t triggered = IF & IE;

    if (halted && (triggered & 0x1F)) {
        halted = false;
        addCycles(4);
    }

    if (!ime || (triggered & 0x1F) == 0) return;

    ime = false;
    uint8_t interrupt = 0;
    if (triggered & 0x01) interrupt = 0;
    else if (triggered & 0x02) interrupt = 1;
    else if (triggered & 0x04) interrupt = 2;
    else if (triggered & 0x08) interrupt = 3;
    else if (triggered & 0x10) interrupt = 4;

    // --- Séquence de 20 cycles exacte ---
    addCycles(8); // Temps de réaction interne

    sp--;
    mmu.write(sp, (pc >> 8) & 0xFF);
    addCycles(4); // L'écriture du byte de poids fort prend 4 cycles

    sp--;
    mmu.write(sp, pc & 0xFF);
    addCycles(4); // L'écriture du byte de poids faible prend 4 cycles

    pc = 0x0040 + (interrupt * 8);
    addCycles(4); // Le saut vers le vecteur prend les 4 derniers cycles

    // On efface le flag à la toute fin
    mmu.write(0xFF0F, mmu.read(0xFF0F) & ~(1 << interrupt));
}