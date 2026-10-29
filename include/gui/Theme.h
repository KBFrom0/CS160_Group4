#ifndef GUI_THEME_H
#define GUI_THEME_H

#include "imgui.h"

// Colors, fonts and sizes of the whole GUI (light + dark).
namespace Theme {
    struct Palette {
        ImVec4 background;   // page background
        ImVec4 card;         // card / panel background
        ImVec4 border;
        ImVec4 text;
        ImVec4 textMuted;
        ImVec4 input;        // text field background
        ImVec4 hover;        // row hover
        ImVec4 navBar;       // dark navigation bar
        ImVec4 navActive;
        ImVec4 navText;
        ImVec4 blue;
        ImVec4 green;
        ImVec4 red;
        ImVec4 orange;
        ImVec4 purple;
        ImVec4 teal;
    };

    // Call once after creating the ImGui context. `dpiScale` = monitor scale (1.0 = 100%).
    void init(float dpiScale);
    // Switch between light and dark (also restyles the charts).
    void apply(bool dark);
    bool isDark();

    const Palette& colors();
    float scale();
    float px(float value);   // value * DPI scale

    ImFont* regularFont();
    ImFont* boldFont();

    // Font sizes (unscaled, pass to ImGui::PushFont).
    const float FONT_SMALL = 13.0f;
    const float FONT_NORMAL = 15.0f;
    const float FONT_LARGE = 18.0f;
    const float FONT_TITLE = 24.0f;
    const float FONT_HUGE = 28.0f;

    ImU32 toU32(const ImVec4& color, float alpha = 1.0f);
}

#endif
