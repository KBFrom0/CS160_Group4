#include "gui/Widgets.h"

#include <cctype>
#include <cstdio>
#include <cmath>
#include <cstring>
#include "imgui_stdlib.h"
#include "implot.h"
#include "IconsFontAwesome6.h"
#include "gui/Theme.h"
#include "DynamicArray.h"
#include "Format.h"

using Theme::px;

namespace {
    float headerBottomY = 0.0f;

    ImVec4 mix(const ImVec4& a, const ImVec4& b, float t) {
        return ImVec4(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t, a.w);
    }

    struct Toast {
        std::string text;
        bool isError = false;
        double startTime = 0.0;
    };
    DynamicArray<Toast> toasts;
    const double TOAST_SECONDS = 3.5;

    const char* MONTH_NAMES[] = { "January", "February", "March", "April", "May", "June", "July",
                                  "August", "September", "October", "November", "December" };
}

namespace Widgets {

// ===================== Layout =====================

void beginCard(const char* id, const ImVec2& size) {
    const Theme::Palette& p = Theme::colors();
    ImGui::PushStyleColor(ImGuiCol_ChildBg, p.card);
    ImGui::PushStyleColor(ImGuiCol_Border, p.border);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(px(20), px(18)));
    ImGuiChildFlags flags = ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding;
    if (size.y <= 0) flags |= ImGuiChildFlags_AutoResizeY;
    ImGui::BeginChild(id, size, flags, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);
}

void endCard() {
    ImGui::EndChild();
}

void pageHeader(const char* title, const char* subtitle, float actionsWidth) {
    float top = ImGui::GetCursorPosY();
    ImGui::BeginGroup();
    ImGui::PushFont(Theme::boldFont(), Theme::FONT_TITLE);
    ImGui::TextUnformatted(title);
    ImGui::PopFont();
    mutedText(subtitle);
    ImGui::EndGroup();
    headerBottomY = ImGui::GetCursorPosY();
    if (actionsWidth > 0.0f) {
        float x = ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - actionsWidth;
        ImGui::SetCursorPos(ImVec2(x, top + px(8)));
    }
}

void headerEnd() {
    ImGui::NewLine();
    if (ImGui::GetCursorPosY() < headerBottomY) ImGui::SetCursorPosY(headerBottomY);
}

void cardTitle(const char* icon, const char* title) {
    const Theme::Palette& p = Theme::colors();
    ImGui::PushFont(Theme::boldFont(), Theme::FONT_LARGE);
    if (icon != nullptr) {
        ImGui::TextColored(p.textMuted, "%s", icon);
        ImGui::SameLine();
    }
    ImGui::TextUnformatted(title);
    ImGui::PopFont();
}

void alignRight(float width) {
    float x = ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - width;
    if (x > ImGui::GetCursorPosX()) ImGui::SetCursorPosX(x);
}

void textCentered(const std::string& text, const ImVec4& color, ImFont* font, float size) {
    if (font != nullptr) ImGui::PushFont(font, size);
    float width = ImGui::CalcTextSize(text.c_str()).x;
    float x = ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - width) * 0.5f;
    if (x > ImGui::GetCursorPosX()) ImGui::SetCursorPosX(x);
    ImGui::TextColored(color, "%s", text.c_str());
    if (font != nullptr) ImGui::PopFont();
}

void mutedText(const std::string& text) {
    ImGui::TextColored(Theme::colors().textMuted, "%s", text.c_str());
}

void emptyState(const char* icon, const std::string& title, const std::string& hint) {
    const Theme::Palette& p = Theme::colors();
    ImGui::Dummy(ImVec2(0, px(18)));
    textCentered(icon, p.border, Theme::regularFont(), 40.0f);
    ImGui::Dummy(ImVec2(0, px(2)));
    textCentered(title, p.text, Theme::boldFont(), Theme::FONT_LARGE);
    if (!hint.empty()) textCentered(hint, p.textMuted);
    ImGui::Dummy(ImVec2(0, px(14)));
}

void todoNote(const std::string& functionName) {
    ImGui::PushFont(nullptr, Theme::FONT_SMALL);
    textCentered(std::string(ICON_FA_CODE "  Waiting for ") + functionName, Theme::colors().orange);
    ImGui::PopFont();
}

// ===================== Dashboard tiles =====================

void statCard(const char* id, const char* icon, const ImVec4& color, const char* label,
              const std::string& value, const ImVec2& size, const char* caption) {
    const Theme::Palette& p = Theme::colors();
    beginCard(id, size);
    float radius = px(24);
    ImVec2 top = ImGui::GetCursorScreenPos();
    float centerX = ImGui::GetWindowPos().x + ImGui::GetWindowWidth() * 0.5f;
    ImVec2 center(centerX, top.y + radius);
    ImDrawList* draw = ImGui::GetWindowDrawList();
    draw->AddCircleFilled(center, radius, Theme::toU32(color));

    ImFont* font = Theme::regularFont();
    float iconSize = Theme::FONT_LARGE * Theme::scale();
    ImVec2 iconDim = font->CalcTextSizeA(iconSize, 1000.0f, 0.0f, icon);
    draw->AddText(font, iconSize, ImVec2(center.x - iconDim.x * 0.5f, center.y - iconDim.y * 0.5f),
                  IM_COL32_WHITE, icon);
    ImGui::Dummy(ImVec2(0, radius * 2 + px(4)));

    textCentered(label, p.text, Theme::regularFont(), Theme::FONT_LARGE);
    textCentered(value, color, Theme::boldFont(), Theme::FONT_HUGE);
    if (caption != nullptr) {
        ImGui::PushFont(nullptr, Theme::FONT_SMALL);
        textCentered(caption, p.textMuted);
        ImGui::PopFont();
    }
    endCard();
}

// ===================== Buttons =====================

bool button(const char* label, const ImVec4& color, const ImVec2& size) {
    ImVec4 white(1, 1, 1, 1);
    ImVec4 black(0, 0, 0, 1);
    ImGui::PushStyleColor(ImGuiCol_Button, color);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, mix(color, white, 0.12f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, mix(color, black, 0.12f));
    ImGui::PushStyleColor(ImGuiCol_Border, color);
    ImGui::PushStyleColor(ImGuiCol_Text, white);
    bool clicked = ImGui::Button(label, size);
    ImGui::PopStyleColor(5);
    return clicked;
}

bool ghostButton(const char* label, const ImVec2& size) {
    const Theme::Palette& p = Theme::colors();
    ImGui::PushStyleColor(ImGuiCol_Button, p.card);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, p.hover);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, p.hover);
    bool clicked = ImGui::Button(label, size);
    ImGui::PopStyleColor(3);
    return clicked;
}

bool iconButton(const char* id, const char* icon, const ImVec4& color, const char* tooltip) {
    const Theme::Palette& p = Theme::colors();
    ImGui::PushID(id);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, p.hover);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, p.hover);
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    bool clicked = ImGui::Button(icon, ImVec2(px(30), px(28)));
    ImGui::PopStyleColor(5);
    if (tooltip != nullptr) ImGui::SetItemTooltip("%s", tooltip);
    ImGui::PopID();
    return clicked;
}

bool segmented(const char* id, const char* const labels[], int count, int& selected) {
    bool changed = false;
    ImGui::PushID(id);
    for (int i = 0; i < count; i++) {
        if (i > 0) ImGui::SameLine(0, px(4));
        ImGui::PushID(i);
        bool clicked = (i == selected) ? button(labels[i], Theme::colors().blue) : ghostButton(labels[i]);
        if (clicked && i != selected) {
            selected = i;
            changed = true;
        }
        ImGui::PopID();
    }
    ImGui::PopID();
    return changed;
}

float buttonWidth(const char* label) {
    return ImGui::CalcTextSize(label, nullptr, true).x + ImGui::GetStyle().FramePadding.x * 2.0f;
}

// ===================== Small visuals =====================

void badge(const char* text, const ImVec4& color) {
    ImVec2 padding(px(8), px(2));
    ImVec2 textSize = ImGui::CalcTextSize(text);
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 size(textSize.x + padding.x * 2, textSize.y + padding.y * 2);
    ImDrawList* draw = ImGui::GetWindowDrawList();
    draw->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), Theme::toU32(color, 0.16f), size.y * 0.5f);
    draw->AddText(ImVec2(pos.x + padding.x, pos.y + padding.y), Theme::toU32(color), text);
    ImGui::Dummy(size);
}

void colorDot(const ImVec4& color) {
    float r = px(5);
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float height = ImGui::GetFrameHeight();   // same height as a framed text line
    ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(pos.x + r, pos.y + height * 0.5f), r, Theme::toU32(color));
    ImGui::Dummy(ImVec2(r * 2, height));
}

// ===================== Form fields =====================

void fieldLabel(const char* label) {
    ImGui::PushFont(Theme::boldFont(), Theme::FONT_SMALL);
    ImGui::TextColored(Theme::colors().textMuted, "%s", label);
    ImGui::PopFont();
}

bool dateInput(const char* id, Date& date, float width) {
    const Theme::Palette& p = Theme::colors();
    bool changed = false;
    ImGui::PushID(id);
    ImGuiStorage* storage = ImGui::GetStateStorage();
    ImGuiID monthKey = ImGui::GetID("##viewMonth");
    ImGuiID yearKey = ImGui::GetID("##viewYear");

    std::string text = std::string(ICON_FA_CALENDAR_DAYS "   ") + date.toString();
    float w = width < 0 ? ImGui::GetContentRegionAvail().x : width;
    ImGui::PushStyleColor(ImGuiCol_Button, p.input);
    ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f, 0.5f));
    if (ImGui::Button(text.c_str(), ImVec2(w, 0))) {
        storage->SetInt(monthKey, date.getMonth());
        storage->SetInt(yearKey, date.getYear());
        ImGui::OpenPopup("##calendar");
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();

    if (ImGui::BeginPopup("##calendar")) {
        int month = storage->GetInt(monthKey, date.getMonth());
        int year = storage->GetInt(yearKey, date.getYear());
        float cell = px(34);
        float gridWidth = cell * 7 + ImGui::GetStyle().ItemSpacing.x * 0.2f * 6;

        if (iconButton("prev", ICON_FA_CHEVRON_LEFT, p.text, "Previous month")) {
            if (--month < 1) { month = 12; year--; }
        }
        ImGui::SameLine();
        std::string title = std::string(MONTH_NAMES[month - 1]) + " " + std::to_string(year);
        float titleWidth = gridWidth - px(30) * 2 - ImGui::GetStyle().ItemSpacing.x * 2;
        ImGui::PushFont(Theme::boldFont(), 0.0f);
        float tw = ImGui::CalcTextSize(title.c_str()).x;
        float startX = ImGui::GetCursorPosX();
        ImGui::SetCursorPosX(startX + (titleWidth - tw) * 0.5f);
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(title.c_str());
        ImGui::PopFont();
        ImGui::SameLine();
        ImGui::SetCursorPosX(startX + titleWidth + ImGui::GetStyle().ItemSpacing.x);
        if (iconButton("next", ICON_FA_CHEVRON_RIGHT, p.text, "Next month")) {
            if (++month > 12) { month = 1; year++; }
        }

        const char* weekdays[] = { "Mo", "Tu", "We", "Th", "Fr", "Sa", "Su" };
        float spacingX = ImGui::GetStyle().ItemSpacing.x * 0.2f;
        float rowX = ImGui::GetCursorPosX();
        float rowY = ImGui::GetCursorPosY();
        for (int i = 0; i < 7; i++) {
            float tw2 = ImGui::CalcTextSize(weekdays[i]).x;
            ImGui::SetCursorPos(ImVec2(rowX + i * (cell + spacingX) + (cell - tw2) * 0.5f, rowY));
            ImGui::TextColored(p.textMuted, "%s", weekdays[i]);
        }
        ImGui::SetCursorPosX(rowX);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(spacingX, px(3)));

        Date today = Date::today();
        int first = dayOfWeek(Date(1, month, year));
        int days = Date::daysInMonth(month, year);
        for (int index = 0; index < 42; index++) {
            int day = index - first + 1;
            if (index % 7 != 0) ImGui::SameLine();
            if (day < 1 || day > days) {
                ImGui::Dummy(ImVec2(cell, cell * 0.85f));
                continue;
            }
            Date d(day, month, year);
            bool isSelected = (d == date);
            bool isToday = (d == today);
            ImGui::PushID(index);
            ImGui::PushStyleColor(ImGuiCol_Border, isToday ? p.blue : ImVec4(0, 0, 0, 0));
            bool clicked = isSelected
                ? button(std::to_string(day).c_str(), p.blue, ImVec2(cell, cell * 0.85f))
                : ghostButton(std::to_string(day).c_str(), ImVec2(cell, cell * 0.85f));
            ImGui::PopStyleColor();
            ImGui::PopID();
            if (clicked) {
                date = d;
                changed = true;
                ImGui::CloseCurrentPopup();
            }
            if (index % 7 == 6 && day >= days) break;
        }
        ImGui::PopStyleVar();

        ImGui::Spacing();
        if (ghostButton("Today", ImVec2(gridWidth, 0))) {
            date = today;
            changed = true;
            ImGui::CloseCurrentPopup();
        }
        storage->SetInt(monthKey, month);
        storage->SetInt(yearKey, year);
        ImGui::EndPopup();
    }
    ImGui::PopID();
    return changed;
}

bool amountInput(const char* id, long long& amount) {
    ImGui::PushID(id);
    ImGui::SetNextItemWidth(-1);
    bool changed = ImGui::InputScalar("##amount", ImGuiDataType_S64, &amount, nullptr, nullptr, "%lld");
    if (amount < 0) amount = 0;
    ImGui::PushFont(nullptr, Theme::FONT_SMALL);
    mutedText(Format::money(amount) + " VND");
    ImGui::PopFont();
    ImGui::PopID();
    return changed;
}

bool textInput(const char* id, std::string& text, const char* hint) {
    ImGui::SetNextItemWidth(-1);
    return ImGui::InputTextWithHint(id, hint, &text);
}

std::string nameOf(const FinanceData& data, MasterData::Kind kind, const std::string& id) {
    int n = MasterData::count(data, kind);
    for (int i = 0; i < n; i++) {
        if (MasterData::idAt(data, kind, i) == id) return MasterData::nameAt(data, kind, i);
    }
    return "";
}

bool masterCombo(const char* id, const FinanceData& data, MasterData::Kind kind,
                 std::string& selectedId, const char* placeholder) {
    bool changed = false;
    std::string preview = nameOf(data, kind, selectedId);
    if (preview.empty()) preview = placeholder;
    ImGui::SetNextItemWidth(-1);
    if (ImGui::BeginCombo(id, preview.c_str())) {
        int n = MasterData::count(data, kind);
        if (n == 0) ImGui::TextDisabled("Nothing here yet. Add items in Master Data.");
        for (int i = 0; i < n; i++) {
            std::string itemId = MasterData::idAt(data, kind, i);
            ImGui::PushID(i);
            if (ImGui::Selectable(MasterData::nameAt(data, kind, i).c_str(), itemId == selectedId)) {
                selectedId = itemId;
                changed = true;
            }
            ImGui::PopID();
        }
        ImGui::EndCombo();
    }
    return changed;
}

bool masterFilter(const char* id, const FinanceData& data, MasterData::Kind kind,
                  std::string& selectedId, const char* allLabel, float width) {
    bool changed = false;
    std::string preview = selectedId.empty() ? allLabel : nameOf(data, kind, selectedId);
    ImGui::SetNextItemWidth(width);
    if (ImGui::BeginCombo(id, preview.c_str())) {
        if (ImGui::Selectable(allLabel, selectedId.empty())) {
            selectedId = "";
            changed = true;
        }
        int n = MasterData::count(data, kind);
        for (int i = 0; i < n; i++) {
            std::string itemId = MasterData::idAt(data, kind, i);
            ImGui::PushID(i);
            if (ImGui::Selectable(MasterData::nameAt(data, kind, i).c_str(), itemId == selectedId)) {
                selectedId = itemId;
                changed = true;
            }
            ImGui::PopID();
        }
        ImGui::EndCombo();
    }
    return changed;
}

// ===================== Modals =====================

bool beginModal(const char* title, float width) {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(px(width), 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(px(24), px(22)));
    bool open = ImGui::BeginPopupModal(title, nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);
    ImGui::PopStyleVar();
    if (!open) return false;

    if (ImGui::IsKeyPressed(ImGuiKey_Escape)) ImGui::CloseCurrentPopup();
    ImGui::PushFont(Theme::boldFont(), Theme::FONT_LARGE + 2.0f);
    const char* visibleEnd = std::strstr(title, "##");   // hide the "###id" part
    ImGui::TextUnformatted(title, visibleEnd);
    ImGui::PopFont();
    ImGui::Dummy(ImVec2(0, px(4)));
    return true;
}

void endModal() {
    ImGui::EndPopup();
}

void formError(const std::string& message) {
    if (message.empty()) return;
    ImGui::PushStyleColor(ImGuiCol_Text, Theme::colors().red);
    ImGui::TextWrapped(ICON_FA_CIRCLE_EXCLAMATION "  %s", message.c_str());
    ImGui::PopStyleColor();
}

// ===================== Notifications =====================

void toast(const std::string& message, bool isError) {
    Toast t;
    t.text = message;
    t.isError = isError;
    t.startTime = ImGui::GetTime();
    toasts.add(t);
}

void drawToasts() {
    double now = ImGui::GetTime();
    for (int i = toasts.size() - 1; i >= 0; i--) {
        if (now - toasts[i].startTime > TOAST_SECONDS) toasts.removeAt(i);
    }
    const Theme::Palette& p = Theme::colors();
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    float y = viewport->WorkPos.y + viewport->WorkSize.y - px(20);
    for (int i = toasts.size() - 1; i >= 0; i--) {
        const Toast& t = toasts[i];
        double age = now - t.startTime;
        float alpha = age > TOAST_SECONDS - 0.4 ? (float)((TOAST_SECONDS - age) / 0.4) : 1.0f;
        ImVec4 color = t.isError ? p.red : p.green;

        ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - px(20), y),
                                ImGuiCond_Always, ImVec2(1.0f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(px(16), px(12)));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, p.card);
        ImGui::PushStyleColor(ImGuiCol_Border, color);
        std::string name = "##toast" + std::to_string(i);
        ImGui::Begin(name.c_str(), nullptr,
            ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
        ImGui::TextColored(color, "%s", t.isError ? ICON_FA_CIRCLE_XMARK : ICON_FA_CIRCLE_CHECK);
        ImGui::SameLine();
        ImGui::PushTextWrapPos(px(360));
        ImGui::TextUnformatted(t.text.c_str());
        ImGui::PopTextWrapPos();
        y -= ImGui::GetWindowHeight() + px(10);
        ImGui::End();
        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar(2);
    }
}

// ===================== Helpers =====================

int dayOfWeek(const Date& date) {
    static const int offsets[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
    int y = date.getYear();
    if (date.getMonth() < 3) y -= 1;
    int sunday0 = (y + y / 4 - y / 100 + y / 400 + offsets[date.getMonth() - 1] + date.getDay()) % 7;
    return (sunday0 + 6) % 7;
}

std::string compactMoney(double value) {
    char buffer[32];
    double a = std::fabs(value);
    if (a < 0.5) return "0";
    if (a >= 1e9)      std::snprintf(buffer, sizeof(buffer), "%.1fB", value / 1e9);
    else if (a >= 1e6) std::snprintf(buffer, sizeof(buffer), "%.1fM", value / 1e6);
    else if (a >= 1e3) std::snprintf(buffer, sizeof(buffer), "%.0fK", value / 1e3);
    else               std::snprintf(buffer, sizeof(buffer), "%.0f", value);
    std::string text = buffer;
    size_t dot = text.find(".0");
    if (dot != std::string::npos && dot + 2 < text.size() && !std::isdigit((unsigned char)text[dot + 2])) {
        text.erase(dot, 2);
    }
    return text;
}

int formatAxisMoney(double value, char* buffer, int size, void* userData) {
    (void)userData;
    return std::snprintf(buffer, size, "%s", compactMoney(value).c_str());
}

void setupBarAxes(const double* positions, int count, const char* const labels[], double maxValue) {
    bool empty = maxValue < 0.5;
    ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoGridLines, empty ? 0 : ImPlotAxisFlags_AutoFit);
    ImPlot::SetupAxisTicks(ImAxis_X1, positions, count, labels);
    ImPlot::SetupAxisLimits(ImAxis_X1, -0.6, count - 0.4, ImPlotCond_Always);
    ImPlot::SetupAxisFormat(ImAxis_Y1, formatAxisMoney);
    if (empty) ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 1000000, ImPlotCond_Always);
}

double maxAbs(const double* values, int count) {
    double result = 0;
    for (int i = 0; i < count; i++) {
        double a = std::fabs(values[i]);
        if (a > result) result = a;
    }
    return result;
}

}
