#ifndef GUI_GUI_H
#define GUI_GUI_H

#include <string>
#include "FinanceData.h"

// Window-independent part of the GUI: app state, page switching, one frame.
namespace Gui {
    enum class Page { DASHBOARD, TRANSACTIONS, RECURRING, MASTER_DATA, STATISTICS, SETTINGS };

    struct AppState {
        FinanceData data;
        std::string dataFile;
        bool demoMode = false;
        bool darkTheme = false;
        Page page = Page::DASHBOARD;
        void* nativeWindow = nullptr;   // HWND, used to color the title bar
    };

    // Loads data + settings and generates this month's recurring transactions.
    void startup(AppState& app, bool demo);
    // Draws one frame (call between ImGui::NewFrame() and ImGui::Render()).
    void frame(AppState& app);
    // Saves everything before the window closes.
    void shutdown(AppState& app);

    void save(AppState& app);
    // Call after a function finished successfully: saves, shows a
    // notification and goes back to the Dashboard (required by the assignment).
    void finishAction(AppState& app, const std::string& message);
    void setTheme(AppState& app, bool dark);
}

#endif
