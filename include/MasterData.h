#ifndef MASTER_DATA_H
#define MASTER_DATA_H

#include <string>
#include "FinanceData.h"

// Expense categories, income sources and wallets: lookup + add/edit/delete.
// Owner: Duyen
namespace MasterData {
    // Index of the item with this id in its list, or -1 if not found.
    int findCategoryIndex(const FinanceData& data, const std::string& id);
    int findSourceIndex(const FinanceData& data, const std::string& id);
    int findWalletIndex(const FinanceData& data, const std::string& id);

    // Display name for an id, or "(unknown)" if not found.
    std::string getCategoryName(const FinanceData& data, const std::string& id);
    std::string getSourceName(const FinanceData& data, const std::string& id);
    std::string getWalletName(const FinanceData& data, const std::string& id);

    // Print a numbered list, let the user pick one, return its id.
    // Returns "" if the list is empty (caller should go back to the Dashboard).
    std::string chooseCategory(const FinanceData& data);
    std::string chooseSource(const FinanceData& data);
    std::string chooseWallet(const FinanceData& data);

    // True if any Income/Expense/RecurringTransaction references this id.
    // Used to block deleting an item that is still in use.
    bool isCategoryInUse(const FinanceData& data, const std::string& id);
    bool isSourceInUse(const FinanceData& data, const std::string& id);
    bool isWalletInUse(const FinanceData& data, const std::string& id);

    // Menu 5: add / edit name / list / delete for all three lists.
    void showMenu(FinanceData& data);
}

#endif
