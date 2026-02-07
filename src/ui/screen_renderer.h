#pragma once
#include "common/types.h"

class ScreenRenderer {
public:
    void render(const uint8_t* framebuffer, int width, int height);
    
private:
    unsigned int texture_id = 0;
};