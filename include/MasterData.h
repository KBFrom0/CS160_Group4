#ifndef MASTER_DATA_H
#define MASTER_DATA_H

#include <string>
#include "FinanceData.h"

// Expense categories, income sources and wallets.
// Logic marked "TODO" in MasterData.cpp: Duyen. The GUI (src/gui) calls these.
namespace MasterData {
    enum class Kind { CATEGORY, SOURCE, WALLET };

    // ===== Logic (TODO: Duyen) =====

    // Index of the item with this id in its list, or -1 if not found.
    int findCategoryIndex(const FinanceData& data, const std::string& id);
    int findSourceIndex(const FinanceData& data, const std::string& id);
    int findWalletIndex(const FinanceData& data, const std::string& id);

    // Display name for an id, or "(unknown)" if not found.
    std::string getCategoryName(const FinanceData& data, const std::string& id);
    std::string getSourceName(const FinanceData& data, const std::string& id);
    std::string getWalletName(const FinanceData& data, const std::string& id);

    // True if any Income/Expense/RecurringTransaction references this id.
    bool isCategoryInUse(const FinanceData& data, const std::string& id);
    bool isSourceInUse(const FinanceData& data, const std::string& id);
    bool isWalletInUse(const FinanceData& data, const std::string& id);

    // Add a new item with a generated ID (CAT001 / SRC001 / WAL001).
    // On failure (empty or duplicate name) return false and explain in `message`.
    bool addItem(FinanceData& data, Kind kind, const std::string& name, std::string& message);
    bool renameItem(FinanceData& data, Kind kind, const std::string& id,
                    const std::string& newName, std::string& message);
    // Must refuse (return false) if the item is still in use.
    bool removeItem(FinanceData& data, Kind kind, const std::string& id, std::string& message);

    // ===== Generic access (done) =====

    int count(const FinanceData& data, Kind kind);
    std::string idAt(const FinanceData& data, Kind kind, int index);
    std::string nameAt(const FinanceData& data, Kind kind, int index);
}

#endif
