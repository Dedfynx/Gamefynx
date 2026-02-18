#include "core/gameboy/GB_MMU.h"
#include "utils/Logger.h"
#include "utils/FileUtils.h"

GB_MMU::GB_MMU() {
    reset();
}

void GB_MMU::reset() {
    memory.fill(0);
    external_RAM.clear();
    //rom_data.clear();
    boot_rom.clear();

    current_ROM_bank = 1;
    current_RAM_bank = 0;
    boot_rom_enabled = false;  // Pas de Boot ROM par défaut

    LOG_DEBUG("GB MMU reset");
}

bool GB_MMU::loadBootROM(const std::string& path) {
    boot_rom = FileUtils::readBinaryFile(path);

    if (boot_rom.empty() || boot_rom.size() != 256) {
        LOG_WARN("Boot ROM invalid or missing: {}", path);
        boot_rom.clear();
        return false;
    }

    boot_rom_enabled = true;
    LOG_INFO("Boot ROM loaded: {}", path);
    return true;
}

bool GB_MMU::loadROM(const std::string& path) {
    rom_data = FileUtils::readBinaryFile(path);

    if (rom_data.empty()) {
        LOG_ERROR("Failed to load ROM: {}", path);
        return false;
    }

    // Affiche les infos de la ROM
    if (rom_data.size() >= 0x0150) {
        // Titre de la ROM (0x0134-0x0143)
        std::string title;
        for (int i = 0x0134; i < 0x0143 && rom_data[i] != 0; ++i) {
            title += static_cast<char>(rom_data[i]);
        }

        uint8_t cartridge_type = rom_data[0x0147];
        uint8_t rom_size_code = rom_data[0x0148];
        uint8_t ram_size_code = rom_data[0x0149];

        LOG_INFO("ROM loaded: {}", path);
        LOG_INFO("  Title: {}", title.empty() ? "Unknown" : title);
        LOG_INFO("  Type: {:#04x}", cartridge_type);
        LOG_INFO("  ROM size: {:#04x}", rom_size_code);
        LOG_INFO("  RAM size: {:#04x}", ram_size_code);
        LOG_INFO("  Total size: {} bytes", rom_data.size());
    } else {
        LOG_INFO("ROM loaded: {} ({} bytes)", path, rom_data.size());
    }

    return true;
}

uint8_t GB_MMU::read(uint16_t addr) const {
    // Boot ROM (0x0000-0x00FF)
    if (boot_rom_enabled && addr < 0x0100 && !boot_rom.empty()) {
        return boot_rom[addr];
    }

    // ROM Bank 0 (0x0000-0x3FFF)
    if (addr < 0x4000) {
        if (addr < rom_data.size()) {
            return rom_data[addr];
        }
        return 0xFF;
    }

    // ROM Bank 1-N (0x4000-0x7FFF) - Switchable
    if (addr >= 0x4000 && addr < 0x8000) {
        uint32_t rom_addr = (current_ROM_bank * 0x4000) + (addr - 0x4000);
        if (rom_addr < rom_data.size()) {
            return rom_data[rom_addr];
        }
        return 0xFF;
    }

    // VRAM (0x8000-0x9FFF)
    if (addr >= 0x8000 && addr < 0xA000) {
        return memory[addr];
    }

    // External RAM (0xA000-0xBFFF) - Switchable
    if (addr >= 0xA000 && addr < 0xC000) {
        uint16_t ram_addr = (current_RAM_bank * 0x2000) + (addr - 0xA000);
        if (ram_addr < external_RAM.size()) {
            return external_RAM[ram_addr];
        }
        return 0xFF;
    }

    // Work RAM Bank 0 (0xC000-0xCFFF)
    // Work RAM Bank 1 (0xD000-0xDFFF)
    if (addr >= 0xC000 && addr < 0xE000) {
        return memory[addr];
    }

    // Echo RAM (0xE000-0xFDFF) - Mirror de WRAM
    if (addr >= 0xE000 && addr < 0xFE00) {
        return memory[addr - 0x2000];
    }

    // OAM (0xFE00-0xFE9F)
    if (addr >= 0xFE00 && addr < 0xFEA0) {
        return memory[addr];
    }

    // Unusable (0xFEA0-0xFEFF)
    if (addr >= 0xFEA0 && addr < 0xFF00) {
        return 0xFF;
    }

    // I/O Registers (0xFF00-0xFF7F)
    if (addr >= 0xFF00 && addr < 0xFF80) {

        if (addr == 0xFF04 && timer) {
            return timer->getDIV();
        }
        // Registre spécial: Boot ROM disable
        if (addr == 0xFF50) {
            return boot_rom_enabled ? 0x00 : 0x01;
        }
        return memory[addr];
    }

    // HRAM (0xFF80-0xFFFE)
    if (addr >= 0xFF80 && addr < 0xFFFF) {
        return memory[addr];
    }

    // IE Register (0xFFFF)
    if (addr == 0xFFFF) {
        return memory[addr];
    }

    return 0xFF;
}

void GB_MMU::write(uint16_t addr, uint8_t data) {
    // ROM (0x0000-0x7FFF) - MBC control
    if (addr < 0x8000) {
        handleMBCWrite(addr, data);
        return;
    }

    // VRAM (0x8000-0x9FFF)
    if (addr >= 0x8000 && addr < 0xA000) {
        static int vramWriteCount = 0;
        if (vramWriteCount++ < 20) {
            LOG_DEBUG("VRAM write: [{:#06x}] = {:#04x}", addr, data);
        }
        memory[addr] = data;
        return;
    }

    // External RAM (0xA000-0xBFFF)
    if (addr >= 0xA000 && addr < 0xC000) {
        uint16_t ram_addr = (current_RAM_bank * 0x2000) + (addr - 0xA000);
        if (ram_addr >= external_RAM.size()) {
            external_RAM.resize(ram_addr + 1, 0);
        }
        external_RAM[ram_addr] = data;
        return;
    }

    // Work RAM (0xC000-0xDFFF)
    if (addr >= 0xC000 && addr < 0xE000) {
        memory[addr] = data;
        return;
    }

    // Echo RAM (0xE000-0xFDFF)
    if (addr >= 0xE000 && addr < 0xFE00) {
        memory[addr - 0x2000] = data;
        return;
    }

    // OAM (0xFE00-0xFE9F)
    if (addr >= 0xFE00 && addr < 0xFEA0) {
        memory[addr] = data;
        return;
    }

    if (addr == 0xFF04) {
        if (timer) {
            timer->resetDIV();
        }
        return;
    }

    // ⚡ TAC (0xFF07) - Gestion spéciale
    if (addr == 0xFF07) {
        if (timer) {
            timer->writeTAC(data);
        }
        return;
    }

    // I/O Registers (0xFF00-0xFF7F)
    if (addr >= 0xFF00 && addr < 0xFF80) {
        // Registre spécial: Boot ROM disable
        memory[addr] = data;

        if (addr == 0xFF50 && data != 0) {
            boot_rom_enabled = false;
            LOG_INFO("Boot ROM disabled");
        }

        return;
    }

    // HRAM (0xFF80-0xFFFE)
    if (addr >= 0xFF80 && addr < 0xFFFF) {
        memory[addr] = data;
        return;
    }

    // IE Register (0xFFFF)
    if (addr == 0xFFFF) {
        memory[addr] = data;
        return;
    }
}

uint16_t GB_MMU::read_word(uint16_t addr) const {
    uint8_t low = read(addr);
    uint8_t high = read(addr + 1);
    return (high << 8) | low;
}

void GB_MMU::write_word(uint16_t addr, uint16_t data) {
    write(addr, data & 0xFF);
    write(addr + 1, (data >> 8) & 0xFF);
}

void GB_MMU::handleMBCWrite(uint16_t addr, uint8_t data) {
    // Pour l'instant, on gère seulement MBC1 de base

    // RAM Enable (0x0000-0x1FFF)
    if (addr < 0x2000) {
        // TODO: Enable/disable RAM
        return;
    }

    // ROM Bank Number (0x2000-0x3FFF)
    if (addr >= 0x2000 && addr < 0x4000) {
        uint8_t bank = data & 0x1F;  // 5 bits
        if (bank == 0) bank = 1;     // Bank 0 pas accessible ici
        current_ROM_bank = bank;
        return;
    }

    // RAM Bank Number (0x4000-0x5FFF)
    if (addr >= 0x4000 && addr < 0x6000) {
        current_RAM_bank = data & 0x03;  // 2 bits
        return;
    }

    // Banking Mode Select (0x6000-0x7FFF)
    if (addr >= 0x6000 && addr < 0x8000) {
        // TODO: ROM/RAM mode select
        return;
    }
}