#ifndef GUI_IMAGES_H
#define GUI_IMAGES_H

#include "imgui.h"

// Loads PNG/JPG files into GPU textures that ImGui can draw.
// Uses Windows Imaging Component (part of Windows), so no extra library is needed.
namespace Images {
    struct Texture {
        ImTextureID id = 0;   // 0 = not loaded
        int width = 0;
        int height = 0;
    };

    // Call once with the ID3D11Device before loading images.
    void init(void* d3dDevice);
    // Returns false (and leaves `out` empty) if the file cannot be read.
    bool load(const char* path, Texture& out);
    // Draws the middle square of the image as a circle.
    void drawCircle(ImDrawList* draw, const Texture& texture, const ImVec2& center, float radius);
}

#endif
