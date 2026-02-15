#include "core/gameboy/GB_PPU.h"
#include "core/gameboy/GB_MMU.h"
#include "utils/Logger.h"

GB_PPU::GB_PPU(GB_MMU& mem) : mmu(mem) {
    reset();
}

void GB_PPU::reset() {
    framebuffer.fill(0xFF);
    scanlineCounter = 0;
    currentScanline = 0;
    mode = PPUMode::OAMScan;
    frameReady = false;
    
    LOG_DEBUG("GB PPU reset");
}

void GB_PPU::step(int cycles) {
    scanlineCounter += cycles;
    
    switch (mode) {
        case PPUMode::OAMScan:
            if (scanlineCounter >= 80) {
                scanlineCounter -= 80;
                mode = PPUMode::Drawing;
            }
            break;
            
        case PPUMode::Drawing:
            if (scanlineCounter >= 172) {
                scanlineCounter -= 172;
                mode = PPUMode::HBlank;
                renderScanline();
            }
            break;
            
        case PPUMode::HBlank:
            if (scanlineCounter >= 204) {
                scanlineCounter -= 204;
                currentScanline++;
                mmu.write(0xFF44, currentScanline);
                
                if (currentScanline == 144) {
                    mode = PPUMode::VBlank;
                    frameReady = true;
                    
                    // Déclenche VBlank interrupt
                    uint8_t IF = mmu.read(0xFF0F);
                    mmu.write(0xFF0F, IF | 0x01);
                } else {
                    mode = PPUMode::OAMScan;
                }
            }
            break;
            
        case PPUMode::VBlank:
            if (scanlineCounter >= 456) {
                scanlineCounter -= 456;
                currentScanline++;
                mmu.write(0xFF44, currentScanline);
                
                if (currentScanline > 153) {
                    currentScanline = 0;
                    mode = PPUMode::OAMScan;
                }
            }
            break;
    }
}

void GB_PPU::renderScanline() {
    renderBackground();
}

void GB_PPU::renderBackground() {
    uint8_t lcdc = mmu.read(0xFF40);
    
    if ((lcdc & 0x80) == 0) {
        return;  // LCD OFF
    }
    
    uint8_t scrollY = mmu.read(0xFF42);
    uint8_t scrollX = mmu.read(0xFF43);
    
    uint16_t tileMapBase = (lcdc & 0x08) ? 0x9C00 : 0x9800;
    bool unsignedTileData = (lcdc & 0x10) != 0;
    uint16_t tileDataBase = unsignedTileData ? 0x8000 : 0x8800;
    
    uint8_t y = currentScanline + scrollY;
    uint8_t tileRow = y / 8;
    uint8_t pixelY = y % 8;
    
    for (int x = 0; x < 160; x++) {
        uint8_t pixelX = (x + scrollX) % 256;
        uint8_t tileCol = pixelX / 8;
        uint8_t tilePixelX = pixelX % 8;
        
        uint16_t tileMapAddr = tileMapBase + (tileRow * 32) + tileCol;
        uint8_t tileIndex = mmu.read(tileMapAddr);
        
        uint16_t tileAddr;
        if (unsignedTileData) {
            tileAddr = tileDataBase + (tileIndex * 16);
        } else {
            int8_t signedIndex = static_cast<int8_t>(tileIndex);
            tileAddr = tileDataBase + ((signedIndex + 128) * 16);
        }
        
        uint8_t byte1 = mmu.read(tileAddr + (pixelY * 2));
        uint8_t byte2 = mmu.read(tileAddr + (pixelY * 2) + 1);
        
        int bitPos = 7 - tilePixelX;
        uint8_t colorBit1 = (byte1 >> bitPos) & 1;
        uint8_t colorBit2 = (byte2 >> bitPos) & 1;
        uint8_t colorId = (colorBit2 << 1) | colorBit1;
        
        uint8_t palette = mmu.read(0xFF47);
        uint8_t color = (palette >> (colorId * 2)) & 0x03;
        
        setPixel(x, currentScanline, color);
    }
}

void GB_PPU::setPixel(int x, int y, uint8_t colorId) {
    if (x < 0 || x >= 160 || y < 0 || y >= 144) return;
    
    static const uint8_t colors[4][3] = {
        {155, 188, 15},   // 0: Blanc
        {139, 172, 15},   // 1: Gris clair
        {48, 98, 48},     // 2: Gris foncé
        {15, 56, 15}      // 3: Noir
    };
    
    int index = (y * 160 + x) * 4;
    framebuffer[index + 0] = colors[colorId][0];
    framebuffer[index + 1] = colors[colorId][1];
    framebuffer[index + 2] = colors[colorId][2];
    framebuffer[index + 3] = 255;
}