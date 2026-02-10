#include "ui/screenRenderer.h"
#include <imgui.h>
#include <SDL3/SDL_opengl.h>
#include <vector>

ScreenRenderer::~ScreenRenderer() {
    if (texture_id != 0) {
        glDeleteTextures(1, &texture_id);
    }
}

void ScreenRenderer::render(const uint8_t* framebuffer, int width, int height) {
    if (!framebuffer) return;
    
    // Crée la texture si nécessaire
    if (texture_id == 0 || texture_width != width || texture_height != height) {
        createTexture(width, height);
    }
    
    // Update texture avec le framebuffer
    updateTexture(framebuffer, width, height);
    
    // Fenêtre ImGui
    ImGui::Begin("CHIP-8 Screen");
    
    // Options de zoom
    ImGui::Text("Scale:");
    ImGui::SameLine();
    if (ImGui::RadioButton("x4", scale == 4)) scale = 4;
    ImGui::SameLine();
    if (ImGui::RadioButton("x8", scale == 8)) scale = 8;
    ImGui::SameLine();
    if (ImGui::RadioButton("x10", scale == 10)) scale = 10;
    
    ImGui::Separator();
    
    // Affiche la texture
    ImVec2 size(static_cast<float>(width * scale), static_cast<float>(height * scale));
    ImGui::Image((void*)static_cast<intptr_t>(texture_id), size);
    
    ImGui::End();
}

void ScreenRenderer::createTexture(int width, int height) {
    // Supprime l'ancienne texture
    if (texture_id != 0) {
        glDeleteTextures(1, &texture_id);
    }
    
    // Crée nouvelle texture
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    
    // Paramètres : nearest neighbor pour pixels nets
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    // Alloue la texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    
    texture_width = width;
    texture_height = height;
}

void ScreenRenderer::updateTexture(const uint8_t* framebuffer, int width, int height) {
    // Convertit monochrome (0/1) en RGBA
    std::vector<uint8_t> rgba_buffer(width * height * 4);
    
    for (int i = 0; i < width * height; ++i) {
        uint8_t pixel = framebuffer[i];
        
        // 0 = noir, 1 = blanc/vert (style retro)
        uint8_t r = pixel ? 100 : 0;
        uint8_t g = pixel ? 255 : 20;
        uint8_t b = pixel ? 100 : 0;
        
        rgba_buffer[i * 4 + 0] = r;
        rgba_buffer[i * 4 + 1] = g;
        rgba_buffer[i * 4 + 2] = b;
        rgba_buffer[i * 4 + 3] = 255;  // Alpha
    }
    
    // Upload vers OpenGL
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
                    GL_RGBA, GL_UNSIGNED_BYTE, rgba_buffer.data());
}