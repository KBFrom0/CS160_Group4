#include "MasterData.h"

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
    // TODO
    (void)data; (void)kind; (void)name;
    message = "Not implemented yet (MasterData::addItem).";
    return false;
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
