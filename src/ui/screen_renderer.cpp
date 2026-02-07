#include "ui/screen_renderer.h"
#include <imgui.h>
#include <GL/gl.h>

void ScreenRenderer::render(const uint8_t* framebuffer, int width, int height) {
    // TODO: créer texture OpenGL et afficher
    ImGui::Begin("Screen");
    ImGui::Text("%dx%d", width, height);
    // ImGui::Image(...) quand texture sera prête
    ImGui::End();
}