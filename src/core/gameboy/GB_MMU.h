//
// Created by Dedfynx on 11/02/2026.
//

#pragma once

#include "common/types.h"
#include "utils/logger.h"
#include <string>
#include <array>
#include <vector>


class GB_MMU
{
public:
    GB_MMU();
    ~GB_MMU() = default;

    void write(uint16_t addr, uint8_t data);
    void write_word(uint16_t addr, uint16_t data);

    uint8_t read(uint16_t addr) const;
    uint16_t read_word(uint16_t addr) const;

    bool loadROM(const std::string& path);
    bool loadBootROM(const std::string& path);

    void reset();

    const uint8_t* getMemoryPtr() const { return memory.data(); }

    void dbg_serial() {
        if (read(0xFF02) == 0x81) {
            char c = static_cast<char>(read(0xFF01));
            LOG_DEBUG("%c", c);
            write(0xFF02, 0x0);
        }
    }

private:
    std::array<uint8_t, 0x10000> memory{};  // 64KB

    std::vector<uint8_t> external_RAM;
    std::vector<uint8_t> rom_data;
    std::vector<uint8_t> boot_rom;

    uint8_t current_ROM_bank = 1;
    uint8_t current_RAM_bank = 0;
    bool boot_rom_enabled = true;
};