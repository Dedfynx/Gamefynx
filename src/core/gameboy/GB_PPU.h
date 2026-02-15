#pragma once
#include "common/Types.h"
#include <array>

class GB_MMU;

class GB_PPU {
public:
    GB_PPU(GB_MMU& mmu);

    void reset();
    void step(int cycles);

    const uint8_t* getFramebuffer() const { return framebuffer.data(); }

    bool isFrameReady() const { return frameReady; }
    void clearFrameReady() { frameReady = false; }

private:
    GB_MMU& mmu;

    // 160x144 pixels * 4 (RGBA)
    std::array<uint8_t, 160 * 144 * 4> framebuffer{};

    bool frameReady = false;

    // LCD state
    int scanlineCounter = 0;
    uint8_t currentScanline = 0;

    enum class PPUMode {
        HBlank = 0,
        VBlank = 1,
        OAMScan = 2,
        Drawing = 3
    };

    PPUMode mode = PPUMode::OAMScan;

    // Rendering
    void renderScanline();
    void renderBackground();
    void setPixel(int x, int y, uint8_t colorId);
};