#ifndef GUI_PAGES_H
#define GUI_PAGES_H

#include <string>
#include "gui/Gui.h"
#include "Transactions.h"

// One function per page. Each page file keeps its own UI state.
namespace Pages {
    void dashboard(Gui::AppState& app);
    void transactions(Gui::AppState& app);
    void recurring(Gui::AppState& app);
    void masterData(Gui::AppState& app);
    void statistics(Gui::AppState& app);
    void settings(Gui::AppState& app);

    // Add / edit transaction dialog (used by Dashboard and Transactions).
    void openAddTransaction(bool isIncome);
    void openEditTransaction(const Gui::AppState& app, const Transactions::TransactionRef& ref);
    void transactionDialog(Gui::AppState& app);

    // Flat copy of one income or expense, for display.
    struct TransactionView {
        bool isIncome = true;
        std::string id;
        Date date;
        std::string refId;        // source id (income) or category id (expense)
        long long amount = 0;
        std::string walletId;
        std::string description;
        std::string recurringId;
    };
    TransactionView viewOf(const FinanceData& data, const Transactions::TransactionRef& ref);
    // Source name (income) or category name (expense).
    std::string refNameOf(const FinanceData& data, const TransactionView& view);
}

#endif
