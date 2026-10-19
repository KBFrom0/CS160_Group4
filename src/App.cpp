#include "App.h"

#include <iostream>
#include "InputHelper.h"
#include "Storage.h"

using namespace InputHelper;

void App::run() {
    Storage::load(data);
    // TODO: generate this month's recurring transactions here, then save.

    while (true) {
        clearScreen();
        showDashboard();

        std::cout << "\n MAIN MENU\n";
        printLine();
        std::cout << " 1. Add income\n"
                  << " 2. Add expense\n"
                  << " 3. View / edit transactions\n"
                  << " 4. Recurring transactions\n"
                  << " 5. Master data (categories, sources, wallets)\n"
                  << " 6. Statistics & reports\n"
                  << " 0. Exit\n";
        printLine();

        int choice = inputInt(" Your choice: ", 0, 6);
        switch (choice) {
            case 1: showNotImplemented("Add income"); break;
            case 2: showNotImplemented("Add expense"); break;
            case 3: showNotImplemented("View / edit transactions"); break;
            case 4: showNotImplemented("Recurring transactions"); break;
            case 5: showNotImplemented("Master data"); break;
            case 6: showNotImplemented("Statistics & reports"); break;
            case 0:
                Storage::save(data);
                std::cout << "Data saved. Goodbye!\n";
                return;
        }
        // Every feature returns here, so the user always lands back on the Dashboard.
        Storage::save(data);
    }
}

void App::showDashboard() {
    printLine('=');
    std::cout << "        PERSONAL FINANCE MANAGER - DASHBOARD\n";
    std::cout << "        Today: " << Date::today().toString() << '\n';
    printLine('=');
    // TODO: show total balance and the balance of each wallet.
    std::cout << " Total balance : (not implemented yet)\n";
    std::cout << " Wallets       : " << data.wallets.size() << '\n';
}

void App::showNotImplemented(const char* feature) {
    std::cout << "\n[" << feature << "] is not implemented yet.\n";
    pause();
}
