#ifndef RECURRING_H
#define RECURRING_H

#include <string>
#include "Date.h"
#include "FinanceData.h"

// Monthly recurring transactions.
// Logic marked "TODO" in Recurring.cpp: Khoa. The GUI (src/gui) calls these.
namespace Recurring {
    // ===== Logic (TODO: Khoa) =====

    // Called once when the app starts. For every recurring item whose
    // [startDate, endDate] covers today's month, add one Income/Expense for
    // this month unless one with the same recurringId already exists in it.
    // Returns how many transactions were added.
    int generateDueTransactions(FinanceData& data, const Date& today);

    // Add `item` with a new ID (REC001...). The id inside `item` is ignored.
    // Return the new ID, or "" on failure.
    std::string addRecurring(FinanceData& data, const RecurringTransaction& item);
    bool updateRecurring(FinanceData& data, int index, const RecurringTransaction& updated);
    bool removeRecurring(FinanceData& data, int index);
}

#endif
