#include "ui/ScreenRenderer.h"
#include <imgui.h>
#include <SDL3/SDL_opengl.h>
#include <vector>

ScreenRenderer::~ScreenRenderer() {
    if (textureID != 0) {
        glDeleteTextures(1, &textureID);
    }
}

void ScreenRenderer::render(const uint8_t* framebuffer, int width, int height) {
    if (!framebuffer) return;
    
    // Crée la texture si nécessaire
    if (textureID == 0 || textureWidth != width || textureHeight != height) {
        createTexture(width, height);
    }
    
    // Update texture avec le framebuffer
    updateTexture(framebuffer, width, height);
    
    // Fenêtre ImGui
    ImGui::Begin("Screen");
    
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
    ImGui::Image((void*)static_cast<intptr_t>(textureID), size);
    
    ImGui::End();
}

void ScreenRenderer::createTexture(int width, int height) {
    // Supprime l'ancienne texture
    if (textureID != 0) {
        glDeleteTextures(1, &textureID);
    }
    
    // Crée nouvelle texture
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    // Paramètres : nearest neighbor pour pixels nets
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    // Alloue la texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    
    textureWidth = width;
    textureHeight = height;
}

void ScreenRenderer::updateTexture(const uint8_t* framebuffer, int width, int height) {
    if (!framebuffer) return;

    glBindTexture(GL_TEXTURE_2D, textureID);

    // ⚡ Détecte le format selon la taille
    bool isMonochrome = (width == 64 && height == 32);  // CHIP-8
    bool isRGBA = (width == 160 && height == 144);      // Game Boy

    if (isMonochrome) {
        // CHIP-8 : Convertit monochrome → RGBA
        std::vector<uint8_t> rgbaBuffer(width * height * 4);

        for (int i = 0; i < width * height; ++i) {
            uint8_t pixel = framebuffer[i];

            uint8_t r = pixel ? 100 : 0;
            uint8_t g = pixel ? 255 : 20;
            uint8_t b = pixel ? 100 : 0;

            rgbaBuffer[i * 4 + 0] = r;
            rgbaBuffer[i * 4 + 1] = g;
            rgbaBuffer[i * 4 + 2] = b;
            rgbaBuffer[i * 4 + 3] = 255;
        }

        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
                        GL_RGBA, GL_UNSIGNED_BYTE, rgbaBuffer.data());
    }
    else if (isRGBA) {
        // Game Boy
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
                        GL_RGBA, GL_UNSIGNED_BYTE, framebuffer);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}