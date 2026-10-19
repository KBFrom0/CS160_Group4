#include "App.h"

#include <iostream>
#include "InputHelper.h"
#include "Storage.h"
#include "MasterData.h"
#include "Balance.h"
#include "Transactions.h"
#include "Recurring.h"
#include "Statistics.h"

using namespace InputHelper;

void App::run() {
    Storage::load(data);
    if (Recurring::generateDueTransactions(data, Date::today()) > 0) {
        Storage::save(data);
    }

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
            case 1: Transactions::addIncome(data); break;
            case 2: Transactions::addExpense(data); break;
            case 3: Transactions::showMenu(data); break;
            case 4: Recurring::showMenu(data); break;
            case 5: MasterData::showMenu(data); break;
            case 6: Statistics::showMenu(data); break;
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
    // TODO (Duyen): print as a neat table once Balance is implemented.
    std::cout << " Total balance : " << formatMoney(Balance::totalBalance(data)) << '\n';
    std::cout << " Wallets:\n";
    for (int i = 0; i < data.wallets.size(); i++) {
        const Wallet& w = data.wallets[i];
        std::cout << "   - " << w.getName() << ": "
                  << formatMoney(Balance::walletBalance(data, w.getId())) << '\n';
    }
    if (data.wallets.isEmpty()) {
        std::cout << "   (no wallets yet - add one in Master data)\n";
    }
}
