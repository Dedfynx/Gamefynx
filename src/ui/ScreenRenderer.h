#pragma once
#include "common/types.h"

class ScreenRenderer {
public:
    ScreenRenderer() = default;
    ~ScreenRenderer();
    
    void render(const uint8_t* framebuffer, int width, int height);
    
    void setScale(int scale) { this->scale = scale; }
    int getScale() const { return scale; }
    
private:
    void createTexture(int width, int height);
    void updateTexture(const uint8_t* framebuffer, int width, int height);
    
    unsigned int textureID = 0;
    int textureWidth = 0;
    int textureHeight = 0;
    int scale = 4;  // Zoom par défaut
};