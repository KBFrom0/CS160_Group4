#ifndef GUI_GUI_H
#define GUI_GUI_H

#include <string>
#include "DynamicArray.h"
#include "FinanceData.h"
#include "User.h"

// Window-independent part of the GUI: app state, page switching, one frame.
namespace Gui {
    enum class Page { DASHBOARD, TRANSACTIONS, RECURRING, MASTER_DATA, STATISTICS, SETTINGS };
    // Screens shown before logging in.
    enum class AuthScreen { WELCOME, LOGIN, REGISTER, RESET };

    struct AppState {
        FinanceData data;               // data of the logged-in account
        std::string dataFile;
        bool darkTheme = false;
        Page page = Page::DASHBOARD;
        void* nativeWindow = nullptr;   // HWND, used to color the title bar

        // Accounts
        DynamicArray<User> users;
        std::string usersFile;          // data/users.dat
        std::string usersDir;           // folder with one data file per account
        bool loggedIn = false;
        std::string username;
        std::string nickname;
        AuthScreen authScreen = AuthScreen::WELCOME;
    };

    // Loads settings and the account list. The finance data is loaded at login.
    void startup(AppState& app);
    // Opens the account users[index]: loads its data and adds this month's
    // recurring transactions, then shows the Dashboard.
    void login(AppState& app, int index);
    // Saves the account's data and goes back to the Get Started screen.
    void logout(AppState& app);
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
