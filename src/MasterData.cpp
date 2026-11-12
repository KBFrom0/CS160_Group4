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
    bool nameExists(const FinanceData& data, Kind kind, const std::string& name) {
        for (int i = 0; i < count(data, kind); i++) {
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
    // TODO
    (void)data; (void)id;
    return false;
}

bool isSourceInUse(const FinanceData& data, const std::string& id) {
    // TODO
    (void)data; (void)id;
    return false;
}

bool isWalletInUse(const FinanceData& data, const std::string& id) {
    // TODO
    (void)data; (void)id;
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
    // TODO
    (void)data; (void)kind; (void)id; (void)newName;
    message = "Not implemented yet (MasterData::renameItem).";
    return false;
}

bool removeItem(FinanceData& data, Kind kind, const std::string& id, std::string& message) {
    // TODO
    (void)data; (void)kind; (void)id;
    message = "Not implemented yet (MasterData::removeItem).";
    return false;
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
