#include "MasterData.h"

#include <cctype>

namespace MasterData {

namespace {
    const std::string UNKNOWN_NAME = "(unknown)";

    // Looks for an id in the list of the given kind. Returns -1 if not found.
    int findIndex(const FinanceData& data, Kind kind, const std::string& id) {
        for (int i = 0; i < count(data, kind); i++) {
            if (idAt(data, kind, i) == id) {
                return i;
            }
        }
        return -1;
    }

    // Removes spaces and tabs at the start and at the end of a name.
    std::string trimSpaces(const std::string& text) {
        size_t first = text.find_first_not_of(" \t");
        if (first == std::string::npos) {
            return "";
        }
        size_t last = text.find_last_not_of(" \t");
        return text.substr(first, last - first + 1);
    }

    std::string toLowerCase(const std::string& text) {
        std::string result = text;
        for (size_t i = 0; i < result.size(); i++) {
            result[i] = (char)std::tolower((unsigned char)result[i]);
        }
        return result;
    }

    // True if an item of this kind already has this name (upper/lower case is ignored).
    // The item at index ignoreIndex is skipped (use -1 to check all items).
    bool nameExists(const FinanceData& data, Kind kind, const std::string& name,
                    int ignoreIndex = -1) {
        for (int i = 0; i < count(data, kind); i++) {
            if (i == ignoreIndex) {
                continue;
            }
            if (toLowerCase(nameAt(data, kind, i)) == toLowerCase(name)) {
                return true;
            }
        }
        return false;
    }

    // Word used in error messages.
    std::string kindLabel(Kind kind) {
        switch (kind) {
            case Kind::CATEGORY: return "category";
            case Kind::SOURCE:   return "income source";
            default:             return "wallet";
        }
    }
}

// ===================== Logic (TODO: Duyen) =====================

int findCategoryIndex(const FinanceData& data, const std::string& id) {
    return findIndex(data, Kind::CATEGORY, id);
}

int findSourceIndex(const FinanceData& data, const std::string& id) {
    return findIndex(data, Kind::SOURCE, id);
}

int findWalletIndex(const FinanceData& data, const std::string& id) {
    return findIndex(data, Kind::WALLET, id);
}

std::string getCategoryName(const FinanceData& data, const std::string& id) {
    int index = findCategoryIndex(data, id);
    if (index == -1) {
        return UNKNOWN_NAME;
    }
    return nameAt(data, Kind::CATEGORY, index);
}

std::string getSourceName(const FinanceData& data, const std::string& id) {
    int index = findSourceIndex(data, id);
    if (index == -1) {
        return UNKNOWN_NAME;
    }
    return nameAt(data, Kind::SOURCE, index);
}

std::string getWalletName(const FinanceData& data, const std::string& id) {
    int index = findWalletIndex(data, id);
    if (index == -1) {
        return UNKNOWN_NAME;
    }
    return nameAt(data, Kind::WALLET, index);
}

bool isCategoryInUse(const FinanceData& data, const std::string& id) {
    // Used by an expense transaction.
    for (int i = 0; i < data.expenses.size(); i++) {
        if (data.expenses[i].getCategoryId() == id) {
            return true;
        }
    }

    // Used by a recurring expense (refId is a category id only for EXPENSE).
    for (int i = 0; i < data.recurrings.size(); i++) {
        if (data.recurrings[i].getType() == TransactionType::EXPENSE &&
            data.recurrings[i].getRefId() == id) {
            return true;
        }
    }
    return false;
}

bool isSourceInUse(const FinanceData& data, const std::string& id) {
    // Used by an income transaction.
    for (int i = 0; i < data.incomes.size(); i++) {
        if (data.incomes[i].getSourceId() == id) {
            return true;
        }
    }

    // Used by a recurring income (refId is a source id only for INCOME).
    for (int i = 0; i < data.recurrings.size(); i++) {
        if (data.recurrings[i].getType() == TransactionType::INCOME &&
            data.recurrings[i].getRefId() == id) {
            return true;
        }
    }
    return false;
}

bool isWalletInUse(const FinanceData& data, const std::string& id) {
    // Used by an income or an expense transaction.
    for (int i = 0; i < data.incomes.size(); i++) {
        if (data.incomes[i].getWalletId() == id) {
            return true;
        }
    }
    for (int i = 0; i < data.expenses.size(); i++) {
        if (data.expenses[i].getWalletId() == id) {
            return true;
        }
    }

    // Used by a recurring transaction (income or expense).
    for (int i = 0; i < data.recurrings.size(); i++) {
        if (data.recurrings[i].getWalletId() == id) {
            return true;
        }
    }
    return false;
}

bool addItem(FinanceData& data, Kind kind, const std::string& name, std::string& message) {
    std::string cleanName = trimSpaces(name);
    if (cleanName.empty()) {
        message = "Name cannot be empty.";
        return false;
    }
    if (nameExists(data, kind, cleanName)) {
        message = "The " + kindLabel(kind) + " \"" + cleanName + "\" already exists.";
        return false;
    }

    switch (kind) {
        case Kind::CATEGORY:
            data.categories.add(ExpenseCategory(
                FinanceData::makeId("CAT", data.nextCategoryNo++, 3), cleanName));
            break;
        case Kind::SOURCE:
            data.sources.add(IncomeSource(
                FinanceData::makeId("SRC", data.nextSourceNo++, 3), cleanName));
            break;
        default:
            data.wallets.add(Wallet(
                FinanceData::makeId("WAL", data.nextWalletNo++, 3), cleanName));
            break;
    }
    message = "";
    return true;
}

bool renameItem(FinanceData& data, Kind kind, const std::string& id,
                const std::string& newName, std::string& message) {
    int index = findIndex(data, kind, id);
    if (index == -1) {
        message = "The " + kindLabel(kind) + " does not exist.";
        return false;
    }

    std::string cleanName = trimSpaces(newName);
    if (cleanName.empty()) {
        message = "Name cannot be empty.";
        return false;
    }
    // The item itself is skipped, so keeping its name or changing only the case is allowed.
    if (nameExists(data, kind, cleanName, index)) {
        message = "The " + kindLabel(kind) + " \"" + cleanName + "\" already exists.";
        return false;
    }

    switch (kind) {
        case Kind::CATEGORY: data.categories[index].setName(cleanName); break;
        case Kind::SOURCE:   data.sources[index].setName(cleanName);    break;
        default:             data.wallets[index].setName(cleanName);    break;
    }
    message = "";
    return true;
}

bool removeItem(FinanceData& data, Kind kind, const std::string& id, std::string& message) {
    int index = findIndex(data, kind, id);
    if (index == -1) {
        message = "The " + kindLabel(kind) + " does not exist.";
        return false;
    }

    bool inUse = false;
    switch (kind) {
        case Kind::CATEGORY: inUse = isCategoryInUse(data, id); break;
        case Kind::SOURCE:   inUse = isSourceInUse(data, id);   break;
        default:             inUse = isWalletInUse(data, id);   break;
    }
    if (inUse) {
        message = "Cannot delete the " + kindLabel(kind) + " \"" + nameAt(data, kind, index) +
                  "\" because it is used by transactions or recurring transactions.";
        return false;
    }

    // The next ID counter is not changed, so a deleted ID is never reused.
    switch (kind) {
        case Kind::CATEGORY: data.categories.removeAt(index); break;
        case Kind::SOURCE:   data.sources.removeAt(index);    break;
        default:             data.wallets.removeAt(index);    break;
    }
    message = "";
    return true;
}

// ===================== Generic access (done) =====================

int count(const FinanceData& data, Kind kind) {
    switch (kind) {
        case Kind::CATEGORY: return data.categories.size();
        case Kind::SOURCE:   return data.sources.size();
        default:             return data.wallets.size();
    }
}

std::string idAt(const FinanceData& data, Kind kind, int index) {
    switch (kind) {
        case Kind::CATEGORY: return data.categories[index].getId();
        case Kind::SOURCE:   return data.sources[index].getId();
        default:             return data.wallets[index].getId();
    }
}

std::string nameAt(const FinanceData& data, Kind kind, int index) {
    switch (kind) {
        case Kind::CATEGORY: return data.categories[index].getName();
        case Kind::SOURCE:   return data.sources[index].getName();
        default:             return data.wallets[index].getName();
    }
}

}
