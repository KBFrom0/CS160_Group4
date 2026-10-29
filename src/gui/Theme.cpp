#include "gui/Theme.h"

#include <cstdio>
#include <cstdlib>
#include <string>
#include "implot.h"
#include "IconsFontAwesome6.h"

namespace {
    float dpi = 1.0f;
    bool dark = false;
    ImFont* regular = nullptr;
    ImFont* bold = nullptr;
    Theme::Palette palette;

    ImVec4 hex(unsigned int rgb, float alpha = 1.0f) {
        return ImVec4(((rgb >> 16) & 0xFF) / 255.0f, ((rgb >> 8) & 0xFF) / 255.0f,
                      (rgb & 0xFF) / 255.0f, alpha);
    }

    bool fileExists(const std::string& path) {
        FILE* f = std::fopen(path.c_str(), "rb");
        if (f == nullptr) return false;
        std::fclose(f);
        return true;
    }

    std::string windowsFont(const char* file) {
        const char* windir = std::getenv("WINDIR");
        return std::string(windir != nullptr ? windir : "C:\\Windows") + "\\Fonts\\" + file;
    }

    // Loads a text font and merges the Font Awesome icons into it.
    ImFont* loadFont(const std::string& path) {
        ImGuiIO& io = ImGui::GetIO();
        ImFont* font = fileExists(path) ? io.Fonts->AddFontFromFileTTF(path.c_str())
                                        : io.Fonts->AddFontDefaultVector();
        const char* iconPath = "assets/fonts/fa-solid-900.ttf";
        if (font != nullptr && fileExists(iconPath)) {
            ImFontConfig config;
            config.MergeMode = true;
            config.PixelSnapH = true;
            config.GlyphOffset = ImVec2(0.0f, 1.0f);
            io.Fonts->AddFontFromFileTTF(iconPath, Theme::FONT_NORMAL, &config);
        }
        return font;
    }

    Theme::Palette lightPalette() {
        Theme::Palette p;
        p.background = hex(0xF3F4F7);
        p.card       = hex(0xFFFFFF);
        p.border     = hex(0xE3E6EC);
        p.text       = hex(0x1F2633);
        p.textMuted  = hex(0x6B7385);
        p.input      = hex(0xF6F7FA);
        p.hover      = hex(0xEEF2FB);
        p.navBar     = hex(0x2E3443);
        p.navActive  = hex(0x232836);
        p.navText    = hex(0xC9CFDB);
        p.blue       = hex(0x3D7BE6);
        p.green      = hex(0x2EA866);
        p.red        = hex(0xE5484D);
        p.orange     = hex(0xEE8A36);
        p.purple     = hex(0x8B5CF6);
        p.teal       = hex(0x14A3A3);
        return p;
    }

    Theme::Palette darkPalette() {
        Theme::Palette p;
        p.background = hex(0x10131A);
        p.card       = hex(0x191E29);
        p.border     = hex(0x2A3142);
        p.text       = hex(0xE6E9F0);
        p.textMuted  = hex(0x9099AC);
        p.input      = hex(0x222836);
        p.hover      = hex(0x232B3B);
        p.navBar     = hex(0x161A24);
        p.navActive  = hex(0x0C0F15);
        p.navText    = hex(0xB4BCCB);
        p.blue       = hex(0x5B93F0);
        p.green      = hex(0x3CC27A);
        p.red        = hex(0xF06368);
        p.orange     = hex(0xF59E4C);
        p.purple     = hex(0xA07BF7);
        p.teal       = hex(0x2BBFBF);
        return p;
    }

    void styleImGui(const Theme::Palette& p) {
        ImGuiStyle style;
        style.WindowPadding = ImVec2(16, 16);
        style.FramePadding = ImVec2(10, 7);
        style.CellPadding = ImVec2(10, 9);
        style.ItemSpacing = ImVec2(10, 10);
        style.ItemInnerSpacing = ImVec2(8, 6);
        style.ScrollbarSize = 12;
        style.GrabMinSize = 10;
        style.WindowRounding = 10;
        style.ChildRounding = 10;
        style.FrameRounding = 7;
        style.PopupRounding = 8;
        style.ScrollbarRounding = 8;
        style.GrabRounding = 6;
        style.TabRounding = 7;
        style.WindowBorderSize = 1;
        style.ChildBorderSize = 1;
        style.PopupBorderSize = 1;
        style.FrameBorderSize = 1;
        style.TabBorderSize = 0;
        style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
        style.SelectableTextAlign = ImVec2(0.0f, 0.5f);

        ImVec4* c = style.Colors;
        ImVec4 accentSoft = p.blue; accentSoft.w = 0.18f;
        ImVec4 accentHover = p.blue; accentHover.w = 0.28f;
        c[ImGuiCol_Text] = p.text;
        c[ImGuiCol_TextDisabled] = p.textMuted;
        c[ImGuiCol_WindowBg] = p.background;
        c[ImGuiCol_ChildBg] = p.card;
        c[ImGuiCol_PopupBg] = p.card;
        c[ImGuiCol_Border] = p.border;
        c[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0);
        c[ImGuiCol_FrameBg] = p.input;
        c[ImGuiCol_FrameBgHovered] = p.hover;
        c[ImGuiCol_FrameBgActive] = p.hover;
        c[ImGuiCol_TitleBg] = p.card;
        c[ImGuiCol_TitleBgActive] = p.card;
        c[ImGuiCol_TitleBgCollapsed] = p.card;
        c[ImGuiCol_MenuBarBg] = p.card;
        c[ImGuiCol_ScrollbarBg] = ImVec4(0, 0, 0, 0);
        c[ImGuiCol_ScrollbarGrab] = p.border;
        c[ImGuiCol_ScrollbarGrabHovered] = p.textMuted;
        c[ImGuiCol_ScrollbarGrabActive] = p.textMuted;
        c[ImGuiCol_CheckMark] = p.blue;
        c[ImGuiCol_SliderGrab] = p.blue;
        c[ImGuiCol_SliderGrabActive] = p.blue;
        c[ImGuiCol_Button] = p.input;
        c[ImGuiCol_ButtonHovered] = p.hover;
        c[ImGuiCol_ButtonActive] = accentSoft;
        c[ImGuiCol_Header] = accentSoft;
        c[ImGuiCol_HeaderHovered] = p.hover;
        c[ImGuiCol_HeaderActive] = accentHover;
        c[ImGuiCol_Separator] = p.border;
        c[ImGuiCol_SeparatorHovered] = p.blue;
        c[ImGuiCol_SeparatorActive] = p.blue;
        c[ImGuiCol_ResizeGrip] = ImVec4(0, 0, 0, 0);
        c[ImGuiCol_ResizeGripHovered] = accentSoft;
        c[ImGuiCol_ResizeGripActive] = accentHover;
        c[ImGuiCol_Tab] = p.card;
        c[ImGuiCol_TabHovered] = p.hover;
        c[ImGuiCol_TabSelected] = accentSoft;
        c[ImGuiCol_TabSelectedOverline] = p.blue;
        c[ImGuiCol_TabDimmed] = p.card;
        c[ImGuiCol_TabDimmedSelected] = accentSoft;
        c[ImGuiCol_TableHeaderBg] = p.input;
        c[ImGuiCol_TableBorderStrong] = p.border;
        c[ImGuiCol_TableBorderLight] = p.border;
        c[ImGuiCol_TableRowBg] = ImVec4(0, 0, 0, 0);
        c[ImGuiCol_TableRowBgAlt] = ImVec4(p.input.x, p.input.y, p.input.z, 0.6f);
        c[ImGuiCol_TextSelectedBg] = accentHover;
        c[ImGuiCol_NavCursor] = p.blue;
        c[ImGuiCol_ModalWindowDimBg] = ImVec4(0.05f, 0.07f, 0.10f, 0.55f);

        style.ScaleAllSizes(dpi);
        style.FontScaleDpi = dpi;
        style.FontSizeBase = Theme::FONT_NORMAL;
        ImGui::GetStyle() = style;
    }

    void stylePlots(const Theme::Palette& p) {
        ImPlotStyle& s = ImPlot::GetStyle();
        if (dark) ImPlot::StyleColorsDark(&s); else ImPlot::StyleColorsLight(&s);
        s.Colors[ImPlotCol_FrameBg] = ImVec4(0, 0, 0, 0);
        s.Colors[ImPlotCol_PlotBg] = ImVec4(0, 0, 0, 0);
        s.Colors[ImPlotCol_PlotBorder] = ImVec4(0, 0, 0, 0);
        s.Colors[ImPlotCol_LegendBg] = p.card;
        s.Colors[ImPlotCol_LegendBorder] = p.border;
        s.Colors[ImPlotCol_LegendText] = p.text;
        s.Colors[ImPlotCol_AxisText] = p.textMuted;
        s.Colors[ImPlotCol_AxisGrid] = ImVec4(p.border.x, p.border.y, p.border.z, 0.9f);
        s.Colors[ImPlotCol_AxisTick] = ImVec4(0, 0, 0, 0);
        s.Colors[ImPlotCol_InlayText] = p.text;
        s.PlotPadding = ImVec2(Theme::px(8), Theme::px(8));
        s.LegendPadding = ImVec2(Theme::px(8), Theme::px(6));
        s.FitPadding = ImVec2(0.05f, 0.12f);

        // Series colors (pie slices etc.) follow the app palette.
        const char* name = dark ? "PFM_Dark" : "PFM_Light";
        ImPlotColormap map = ImPlot::GetColormapIndex(name);
        if (map == -1) {
            const ImVec4 series[] = { p.blue, p.orange, p.green, p.purple, p.teal, p.red };
            map = ImPlot::AddColormap(name, series, 6);
        }
        s.Colormap = map;
    }
}

namespace Theme {

void init(float dpiScale) {
    dpi = dpiScale > 0 ? dpiScale : 1.0f;
    regular = loadFont(windowsFont("segoeui.ttf"));
    bold = loadFont(windowsFont("seguisb.ttf"));
    if (bold == nullptr) bold = regular;
}

void apply(bool useDark) {
    dark = useDark;
    palette = dark ? darkPalette() : lightPalette();
    styleImGui(palette);
    stylePlots(palette);
}

bool isDark() { return dark; }
const Palette& colors() { return palette; }
float scale() { return dpi; }
float px(float value) { return value * dpi; }
ImFont* regularFont() { return regular; }
ImFont* boldFont() { return bold; }

ImU32 toU32(const ImVec4& color, float alpha) {
    return ImGui::ColorConvertFloat4ToU32(ImVec4(color.x, color.y, color.z, color.w * alpha));
}

}
