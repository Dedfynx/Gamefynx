#pragma once
#include "common/types.h"

class ScreenRenderer {
public:
    ScreenRenderer();
    ~ScreenRenderer();
    
    void render(const uint8_t* framebuffer, int width, int height);
    
    void setScale(int scale) { this->scale = scale; }
    int getScale() const { return scale; }
    
private:
    void createTexture(int width, int height);
    void updateTexture(const uint8_t* framebuffer, int width, int height);
    
    unsigned int texture_id = 0;
    int texture_width = 0;
    int texture_height = 0;
    int scale = 8;  // Zoom par défaut
};