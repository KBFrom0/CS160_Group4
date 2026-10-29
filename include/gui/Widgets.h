#ifndef GUI_WIDGETS_H
#define GUI_WIDGETS_H

#include <string>
#include "imgui.h"
#include "Date.h"
#include "FinanceData.h"
#include "MasterData.h"

// Reusable building blocks so every page looks the same.
namespace Widgets {
    // ---- Layout ----
    // White rounded panel. size.y = 0 -> grows to fit its content.
    void beginCard(const char* id, const ImVec2& size = ImVec2(0, 0));
    void endCard();
    // Page title + gray subtitle. If actionsWidth > 0 the cursor is moved to the
    // top-right corner so the caller can draw buttons there; call headerEnd() after them.
    void pageHeader(const char* title, const char* subtitle, float actionsWidth = 0.0f);
    void headerEnd();
    void cardTitle(const char* icon, const char* title);
    // Moves the cursor so that a block of `width` px ends at the right edge.
    void alignRight(float width);
    void textCentered(const std::string& text, const ImVec4& color, ImFont* font = nullptr, float size = 0.0f);
    void mutedText(const std::string& text);
    void emptyState(const char* icon, const std::string& title, const std::string& hint = "");
    // Small gray note showing which TODO function a screen is waiting for.
    void todoNote(const std::string& functionName);

    // ---- Dashboard tiles ----
    void statCard(const char* id, const char* icon, const ImVec4& color, const char* label,
                  const std::string& value, const ImVec2& size, const char* caption = nullptr);

    // ---- Buttons ----
    bool button(const char* label, const ImVec4& color, const ImVec2& size = ImVec2(0, 0));
    bool ghostButton(const char* label, const ImVec2& size = ImVec2(0, 0));
    bool iconButton(const char* id, const char* icon, const ImVec4& color, const char* tooltip);
    // Row of toggle buttons. Returns true when the selection changed.
    bool segmented(const char* id, const char* const labels[], int count, int& selected);
    float buttonWidth(const char* label);

    // ---- Small visuals ----
    void badge(const char* text, const ImVec4& color);
    void colorDot(const ImVec4& color);

    // ---- Form fields (label above, full width) ----
    void fieldLabel(const char* label);
    bool dateInput(const char* id, Date& date, float width = -1.0f);
    bool amountInput(const char* id, long long& amount);
    bool textInput(const char* id, std::string& text, const char* hint = "");
    // Combo with the names of a master data list. `id` is the selected ID ("" = none).
    bool masterCombo(const char* id, const FinanceData& data, MasterData::Kind kind,
                     std::string& selectedId, const char* placeholder);
    // Same, but with an "All" first entry (selectedId = "" means all).
    bool masterFilter(const char* id, const FinanceData& data, MasterData::Kind kind,
                      std::string& selectedId, const char* allLabel, float width);
    std::string nameOf(const FinanceData& data, MasterData::Kind kind, const std::string& id);

    // ---- Modals ----
    // Centered modal of fixed width. Returns true while it is open (call endModal()).
    bool beginModal(const char* title, float width);
    void endModal();
    // Red message under a form.
    void formError(const std::string& message);

    // ---- Notifications (bottom-right) ----
    void toast(const std::string& message, bool isError = false);
    void drawToasts();

    // ---- Helpers ----
    int dayOfWeek(const Date& date);   // 0 = Monday ... 6 = Sunday
    // Short money text for chart axes: 1500000 -> "1.5M".
    std::string compactMoney(double value);
    int formatAxisMoney(double value, char* buffer, int size, void* userData);
    // Axes for a bar chart with one bar (group) per label. `maxValue` = largest
    // absolute value shown; when it is 0 a fixed 0..1M scale is used.
    void setupBarAxes(const double* positions, int count, const char* const labels[], double maxValue);
    double maxAbs(const double* values, int count);
}

#endif
