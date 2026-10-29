#include "gui/Gui.h"

#include <fstream>
#include <windows.h>
#include <dwmapi.h>
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include "gui/Pages.h"
#include "gui/Theme.h"
#include "gui/Widgets.h"
#include "DemoData.h"
#include "Recurring.h"
#include "Storage.h"

using Theme::px;

namespace {
    const char* SETTINGS_FILE = "data/settings.cfg";
    int requestedTheme = -1;   // -1 = no change, 0 = light, 1 = dark (applied at frame start)

    void applyTheme(Gui::AppState& app, bool dark);

    void loadSettings(Gui::AppState& app) {
        std::ifstream in(SETTINGS_FILE);
        std::string line;
        while (std::getline(in, line)) {
            if (line == "theme=dark") app.darkTheme = true;
            if (line == "theme=light") app.darkTheme = false;
        }
    }

    void saveSettings(const Gui::AppState& app) {
        std::ofstream out(SETTINGS_FILE);
        out << "theme=" << (app.darkTheme ? "dark" : "light") << '\n';
    }

    void colorTitleBar(const Gui::AppState& app) {
        if (app.nativeWindow == nullptr) return;
        BOOL dark = app.darkTheme ? TRUE : FALSE;
        const DWORD DWMWA_USE_IMMERSIVE_DARK_MODE_VALUE = 20;
        DwmSetWindowAttribute((HWND)app.nativeWindow, DWMWA_USE_IMMERSIVE_DARK_MODE_VALUE, &dark, sizeof(dark));
    }

    struct NavItem {
        Gui::Page page;
        const char* icon;
        const char* label;
    };

    const NavItem NAV_ITEMS[] = {
        { Gui::Page::DASHBOARD,    ICON_FA_GAUGE_HIGH,    "Dashboard" },
        { Gui::Page::TRANSACTIONS, ICON_FA_RIGHT_LEFT,    "Transactions" },
        { Gui::Page::RECURRING,    ICON_FA_ARROWS_ROTATE, "Recurring" },
        { Gui::Page::MASTER_DATA,  ICON_FA_LAYER_GROUP,   "Master Data" },
        { Gui::Page::STATISTICS,   ICON_FA_CHART_PIE,     "Statistics" },
        { Gui::Page::SETTINGS,     ICON_FA_GEAR,          "Settings" },
    };
    const int NAV_COUNT = 6;

    void topBar(Gui::AppState& app) {
        const Theme::Palette& p = Theme::colors();
        float height = px(58);
        ImVec2 origin = ImGui::GetCursorScreenPos();
        float width = ImGui::GetContentRegionAvail().x;
        ImDrawList* draw = ImGui::GetWindowDrawList();
        draw->AddRectFilled(origin, ImVec2(origin.x + width, origin.y + height), Theme::toU32(p.card));
        draw->AddLine(ImVec2(origin.x, origin.y + height - 1), ImVec2(origin.x + width, origin.y + height - 1),
                      Theme::toU32(p.border));

        // Logo
        float logo = px(34);
        ImVec2 logoMin(origin.x + px(24), origin.y + (height - logo) * 0.5f);
        draw->AddRectFilled(logoMin, ImVec2(logoMin.x + logo, logoMin.y + logo), Theme::toU32(p.blue), px(9));
        ImFont* font = Theme::regularFont();
        float iconSize = Theme::FONT_LARGE * Theme::scale();
        ImVec2 iconDim = font->CalcTextSizeA(iconSize, 1000.0f, 0.0f, ICON_FA_WALLET);
        draw->AddText(font, iconSize, ImVec2(logoMin.x + (logo - iconDim.x) * 0.5f, logoMin.y + (logo - iconDim.y) * 0.5f),
                      IM_COL32_WHITE, ICON_FA_WALLET);

        ImGui::SetCursorScreenPos(ImVec2(logoMin.x + logo + px(12), origin.y + px(10)));
        ImGui::BeginGroup();
        ImGui::PushFont(Theme::boldFont(), Theme::FONT_LARGE);
        ImGui::TextUnformatted("Personal Finance Manager");
        ImGui::PopFont();
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - px(2));
        ImGui::PushFont(nullptr, Theme::FONT_SMALL);
        Widgets::mutedText("CS160 · Group 4");
        ImGui::PopFont();
        ImGui::EndGroup();

        // Right side: demo badge, date, theme toggle
        std::string date = std::string(ICON_FA_CALENDAR "  ") + Date::today().toString();
        float right = origin.x + width - px(24);
        float toggleWidth = px(30);
        float dateWidth = ImGui::CalcTextSize(date.c_str()).x;
        float y = origin.y + (height - px(28)) * 0.5f;

        ImGui::SetCursorScreenPos(ImVec2(right - toggleWidth, y));
        if (Widgets::iconButton("theme", app.darkTheme ? ICON_FA_SUN : ICON_FA_MOON, p.textMuted,
                                app.darkTheme ? "Switch to light mode" : "Switch to dark mode")) {
            Gui::setTheme(app, !app.darkTheme);
        }
        ImGui::SetCursorScreenPos(ImVec2(right - toggleWidth - px(18) - dateWidth,
                                         origin.y + (height - ImGui::GetTextLineHeight()) * 0.5f));
        Widgets::mutedText(date);
        if (app.demoMode) {
            const char* text = ICON_FA_FLASK "  DEMO DATA";
            float badgeWidth = ImGui::CalcTextSize(text).x + px(16);
            ImGui::SetCursorScreenPos(ImVec2(right - toggleWidth - px(18) - dateWidth - px(18) - badgeWidth,
                                             origin.y + (height - ImGui::GetTextLineHeight() - px(4)) * 0.5f));
            Widgets::badge(text, p.orange);
        }
        ImGui::SetCursorScreenPos(ImVec2(origin.x, origin.y + height));
    }

    void navBar(Gui::AppState& app) {
        const Theme::Palette& p = Theme::colors();
        float height = px(46);
        ImVec2 origin = ImGui::GetCursorScreenPos();
        float width = ImGui::GetContentRegionAvail().x;
        ImDrawList* draw = ImGui::GetWindowDrawList();
        draw->AddRectFilled(origin, ImVec2(origin.x + width, origin.y + height), Theme::toU32(p.navBar));

        float x = origin.x + px(24);
        for (int i = 0; i < NAV_COUNT; i++) {
            const NavItem& item = NAV_ITEMS[i];
            std::string label = std::string(item.icon) + "   " + item.label;
            float itemWidth = ImGui::CalcTextSize(label.c_str()).x + px(36);
            ImVec2 min(x, origin.y);
            ImVec2 max(x + itemWidth, origin.y + height);

            ImGui::SetCursorScreenPos(min);
            ImGui::PushID(i);
            bool clicked = ImGui::InvisibleButton("##nav", ImVec2(itemWidth, height));
            bool hovered = ImGui::IsItemHovered();
            ImGui::PopID();
            if (clicked) app.page = item.page;

            bool active = (app.page == item.page);
            if (active) {
                draw->AddRectFilled(min, max, Theme::toU32(p.navActive));
                draw->AddRectFilled(ImVec2(min.x, max.y - px(3)), max, Theme::toU32(p.blue));
            } else if (hovered) {
                draw->AddRectFilled(min, max, Theme::toU32(p.navActive, 0.5f));
            }
            ImU32 textColor = (active || hovered) ? IM_COL32_WHITE : Theme::toU32(p.navText);
            float textY = origin.y + (height - ImGui::GetTextLineHeight()) * 0.5f;
            draw->AddText(ImVec2(x + px(18), textY), textColor, label.c_str());
            x += itemWidth;
        }
        ImGui::SetCursorScreenPos(ImVec2(origin.x, origin.y + height));
        ImGui::Dummy(ImVec2(0, 0));
    }

    void content(Gui::AppState& app) {
        const Theme::Palette& p = Theme::colors();
        float available = ImGui::GetContentRegionAvail().x;
        float maxWidth = px(1360);
        float sidePadding = available > maxWidth + px(56) ? (available - maxWidth) * 0.5f : px(28);

        ImGui::PushStyleColor(ImGuiCol_ChildBg, p.background);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(sidePadding, px(24)));
        ImGui::BeginChild("##content", ImVec2(0, 0), ImGuiChildFlags_AlwaysUseWindowPadding);
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();

        switch (app.page) {
            case Gui::Page::DASHBOARD:    Pages::dashboard(app); break;
            case Gui::Page::TRANSACTIONS: Pages::transactions(app); break;
            case Gui::Page::RECURRING:    Pages::recurring(app); break;
            case Gui::Page::MASTER_DATA:  Pages::masterData(app); break;
            case Gui::Page::STATISTICS:   Pages::statistics(app); break;
            case Gui::Page::SETTINGS:     Pages::settings(app); break;
        }
        ImGui::Dummy(ImVec2(0, px(12)));
        ImGui::EndChild();
    }
}

namespace Gui {

void startup(AppState& app, bool demo) {
    app.demoMode = demo;
    app.dataFile = demo ? "data/demo.dat" : Storage::DEFAULT_FILE;
    CreateDirectoryA("data", nullptr);

    loadSettings(app);
    applyTheme(app, app.darkTheme);

    Storage::load(app.data, app.dataFile);
    if (demo && app.data.wallets.isEmpty()) {
        DemoData::fill(app.data);
    }

    Date today = Date::today();
    int added = Recurring::generateDueTransactions(app.data, today);
    if (added > 0) {
        save(app);
        Widgets::toast(std::to_string(added) + " recurring transaction(s) were added for "
                       + today.toString().substr(3) + ".");
    }
}

void frame(AppState& app) {
    if (requestedTheme != -1) {
        applyTheme(app, requestedTheme == 1);
        requestedTheme = -1;
    }
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    ImGui::Begin("##root", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::PopStyleVar(4);

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    topBar(app);
    navBar(app);
    ImGui::PopStyleVar();
    content(app);
    ImGui::End();

    Pages::transactionDialog(app);
    Widgets::drawToasts();
}

void shutdown(AppState& app) {
    save(app);
    saveSettings(app);
}

void save(AppState& app) {
    Storage::save(app.data, app.dataFile);
}

void finishAction(AppState& app, const std::string& message) {
    save(app);
    Widgets::toast(message);
    app.page = Page::DASHBOARD;
}

void setTheme(AppState& app, bool dark) {
    (void)app;
    requestedTheme = dark ? 1 : 0;
}

}

namespace {
    void applyTheme(Gui::AppState& app, bool dark) {
        app.darkTheme = dark;
        Theme::apply(dark);
        colorTitleBar(app);
        saveSettings(app);
    }
}
