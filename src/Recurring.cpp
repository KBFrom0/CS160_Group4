#include "Recurring.h"

namespace {

// Every referenced ID must exist, and the amount and period must make sense.
bool isValidItem(const FinanceData& data, const RecurringTransaction& item) {
    if (item.getAmount() <= 0 || !item.getStartDate().isValid()) {
        return false;
    }

    if (item.getHasEndDate() &&
        (!item.getEndDate().isValid() || item.getEndDate() < item.getStartDate())) {
        return false;
    }

    bool refFound = false;
    if (item.getType() == TransactionType::INCOME) {
        for (int i = 0; i < data.sources.size(); i++) {
            if (data.sources[i].getId() == item.getRefId()) {
                refFound = true;
                break;
            }
        }
    } else {
        for (int i = 0; i < data.categories.size(); i++) {
            if (data.categories[i].getId() == item.getRefId()) {
                refFound = true;
                break;
            }
        }
    }

    if (!refFound) {
        return false;
    }

    for (int i = 0; i < data.wallets.size(); i++) {
        if (data.wallets[i].getId() == item.getWalletId()) {
            return true;
        }
    }
    return false;
}

// Same content as `item`, but with the given ID.
RecurringTransaction withId(const std::string& id, const RecurringTransaction& item) {
    return RecurringTransaction(id, item.getType(), item.getRefId(), item.getAmount(),
                                item.getWalletId(), item.getDescription(),
                                item.getStartDate(), item.getHasEndDate(), item.getEndDate());
}

}

namespace Recurring {

// ===================== Logic (TODO: Khoa) =====================

int generateDueTransactions(FinanceData& data, const Date& today) {
    // TODO
    (void)data; (void)today;
    return 0;
}

std::string addRecurring(FinanceData& data, const RecurringTransaction& item) {
    if (!isValidItem(data, item)) {
        return "";
    }

    std::string id = FinanceData::makeId("REC", data.nextRecurringNo, 3);
    data.recurrings.add(withId(id, item));
    data.nextRecurringNo++;

    return id;
}

bool updateRecurring(FinanceData& data, int index, const RecurringTransaction& updated) {
    if (index < 0 || index >= data.recurrings.size() || !isValidItem(data, updated)) {
        return false;
    }

    // The ID never changes: generated transactions refer to it.
    data.recurrings[index] = withId(data.recurrings[index].getId(), updated);
    return true;
}

bool removeRecurring(FinanceData& data, int index) {
    if (index < 0 || index >= data.recurrings.size()) {
        return false;
    }

    // Transactions already generated from this item are kept.
    data.recurrings.removeAt(index);
    return true;
}

}
