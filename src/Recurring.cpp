#include "Recurring.h"

#include "MasterData.h"

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

    int refIndex = item.getType() == TransactionType::INCOME
                       ? MasterData::findSourceIndex(data, item.getRefId())
                       : MasterData::findCategoryIndex(data, item.getRefId());
    if (refIndex == -1) {
        return false;
    }

    return MasterData::findWalletIndex(data, item.getWalletId()) != -1;
}

// Same content as `item`, but with the given ID.
RecurringTransaction withId(const std::string& id, const RecurringTransaction& item) {
    return RecurringTransaction(id, item.getType(), item.getRefId(), item.getAmount(),
                                item.getWalletId(), item.getDescription(),
                                item.getStartDate(), item.getHasEndDate(), item.getEndDate());
}

// Months counted from year 0, so two dates can be compared by month only.
int monthNumber(const Date& date) {
    return date.getYear() * 12 + date.getMonth();
}

bool sameMonth(const Date& a, const Date& b) {
    return monthNumber(a) == monthNumber(b);
}

// True if a transaction from this recurring item already exists in the month of `today`.
bool alreadyGenerated(const FinanceData& data, const std::string& recurringId, const Date& today) {
    for (int i = 0; i < data.incomes.size(); i++) {
        if (data.incomes[i].getRecurringId() == recurringId &&
            sameMonth(data.incomes[i].getDate(), today)) {
            return true;
        }
    }
    for (int i = 0; i < data.expenses.size(); i++) {
        if (data.expenses[i].getRecurringId() == recurringId &&
            sameMonth(data.expenses[i].getDate(), today)) {
            return true;
        }
    }
    return false;
}

}

namespace Recurring {

// ===================== Logic (TODO: Khoa) =====================

int generateDueTransactions(FinanceData& data, const Date& today) {
    if (!today.isValid()) {
        return 0;
    }

    int added = 0;
    for (int i = 0; i < data.recurrings.size(); i++) {
        const RecurringTransaction& item = data.recurrings[i];

        // Only months inside [start month, end month] are due.
        if (monthNumber(today) < monthNumber(item.getStartDate())) {
            continue;
        }
        if (item.getHasEndDate() && monthNumber(today) > monthNumber(item.getEndDate())) {
            continue;
        }
        if (!isValidItem(data, item) || alreadyGenerated(data, item.getId(), today)) {
            continue;
        }

        // Same day as the start date; 31 becomes the last day of a shorter month.
        int day = item.getStartDate().getDay();
        int lastDay = Date::daysInMonth(today.getMonth(), today.getYear());
        if (day > lastDay) {
            day = lastDay;
        }
        Date date(day, today.getMonth(), today.getYear());

        if (item.getType() == TransactionType::INCOME) {
            std::string id = FinanceData::makeId("INC", data.nextIncomeNo, 4);
            data.incomes.add(Income(id, date, item.getRefId(), item.getAmount(),
                                    item.getWalletId(), item.getDescription(), item.getId()));
            data.nextIncomeNo++;
        } else {
            std::string id = FinanceData::makeId("EXP", data.nextExpenseNo, 4);
            data.expenses.add(Expense(id, date, item.getRefId(), item.getAmount(),
                                      item.getWalletId(), item.getDescription(), item.getId()));
            data.nextExpenseNo++;
        }
        added++;
    }
    return added;
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
