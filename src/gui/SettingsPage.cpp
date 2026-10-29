#include "gui/Pages.h"

#include "imgui.h"
#include "IconsFontAwesome6.h"
#include "gui/Theme.h"
#include "gui/Widgets.h"

using Theme::px;

namespace {
    void row(const char* label, const std::string& value) {
        ImGui::TableNextRow(ImGuiTableRowFlags_None, px(34));
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        Widgets::mutedText(label);
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(value.c_str());
    }
}

namespace Pages {

void settings(Gui::AppState& app) {
    const Theme::Palette& p = Theme::colors();
    const FinanceData& data = app.data;
    Widgets::pageHeader("Settings", "Appearance, data file and information about the app");
    ImGui::Spacing();

    float gap = px(18);
    float half = (ImGui::GetContentRegionAvail().x - gap) * 0.5f;

    // Appearance
    Widgets::beginCard("##appearance", ImVec2(half, px(200)));
    Widgets::cardTitle(ICON_FA_PALETTE, "Appearance");
    Widgets::mutedText("Choose how the app looks. Saved for next time.");
    ImGui::Spacing();
    const char* themes[] = { ICON_FA_SUN "  Light", ICON_FA_MOON "  Dark" };
    int selected = app.darkTheme ? 1 : 0;
    if (Widgets::segmented("theme", themes, 2, selected)) Gui::setTheme(app, selected == 1);
    Widgets::endCard();

    ImGui::SameLine(0, gap);

    // Data
    Widgets::beginCard("##dataInfo", ImVec2(0, px(200)));
    Widgets::cardTitle(ICON_FA_DATABASE, "Data");
    if (ImGui::BeginTable("##data", 2, ImGuiTableFlags_SizingStretchProp)) {
        ImGui::TableSetupColumn("k", ImGuiTableColumnFlags_WidthFixed, px(150));
        ImGui::TableSetupColumn("v", ImGuiTableColumnFlags_WidthStretch);
        row("Data file", app.dataFile + (app.demoMode ? "   (demo)" : ""));
        row("Saved", "Automatically after every change and on exit");
        row("Records", std::to_string(data.incomes.size()) + " incomes, " +
                       std::to_string(data.expenses.size()) + " expenses, " +
                       std::to_string(data.recurrings.size()) + " recurring");
        ImGui::EndTable();
    }
    Widgets::endCard();

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + gap - ImGui::GetStyle().ItemSpacing.y);

    // About
    Widgets::beginCard("##about");
    Widgets::cardTitle(ICON_FA_CIRCLE_INFO, "About");
    Widgets::mutedText("Personal Finance Manager - Final project, Introduction to Programming (CS160), FIT - VNU-HCMUS");
    ImGui::Spacing();
    if (ImGui::BeginTable("##team", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_PadOuterX)) {
        ImGui::TableSetupColumn("No.", ImGuiTableColumnFlags_WidthFixed, px(60));
        ImGui::TableSetupColumn("Student ID", ImGuiTableColumnFlags_WidthFixed, px(140));
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();
        const char* members[][3] = {
            { "2",  "23125084", "Nguyễn Đăng Khoa" },
            { "3",  "24125028", "Trương Thị Mỹ Duyên" },
            { "28", "26125042", "Huỳnh Phước Trung Kiên" },
            { "36", "26125060", "Dương Minh Quân" },
        };
        for (int i = 0; i < 4; i++) {
            ImGui::TableNextRow(ImGuiTableRowFlags_None, px(32));
            for (int c = 0; c < 3; c++) {
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted(members[i][c]);
            }
        }
        ImGui::EndTable();
    }
    ImGui::Spacing();
    ImGui::PushFont(nullptr, Theme::FONT_SMALL);
    Widgets::mutedText("Built with C++17, Dear ImGui, ImPlot and Font Awesome.");
    ImGui::PopFont();
    Widgets::endCard();
    (void)p;
}

}
