#include "MasterData.h"

namespace MasterData {

// ===================== Logic (TODO: Duyen) =====================

int findCategoryIndex(const FinanceData& data, const std::string& id) {
    // TODO
    (void)data; (void)id;
    return -1;
}

int findSourceIndex(const FinanceData& data, const std::string& id) {
    // TODO
    (void)data; (void)id;
    return -1;
}

int findWalletIndex(const FinanceData& data, const std::string& id) {
    // TODO
    (void)data; (void)id;
    return -1;
}

std::string getCategoryName(const FinanceData& data, const std::string& id) {
    // TODO
    (void)data; (void)id;
    return "(unknown)";
}

std::string getSourceName(const FinanceData& data, const std::string& id) {
    // TODO
    (void)data; (void)id;
    return "(unknown)";
}

std::string getWalletName(const FinanceData& data, const std::string& id) {
    // TODO
    (void)data; (void)id;
    return "(unknown)";
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
