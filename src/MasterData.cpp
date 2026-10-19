#include "MasterData.h"

#include "InputHelper.h"

namespace MasterData {

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

std::string chooseCategory(const FinanceData& data) {
    // TODO
    (void)data;
    return "";
}

std::string chooseSource(const FinanceData& data) {
    // TODO
    (void)data;
    return "";
}

std::string chooseWallet(const FinanceData& data) {
    // TODO
    (void)data;
    return "";
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

void showMenu(FinanceData& data) {
    // TODO
    (void)data;
    InputHelper::showNotImplemented("Master data");
}

}
