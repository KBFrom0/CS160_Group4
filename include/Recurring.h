#ifndef RECURRING_H
#define RECURRING_H

#include "Date.h"
#include "FinanceData.h"

// Monthly recurring transactions.
// Owner: Khoa
namespace Recurring {
    // Called once when the app starts. For every recurring item whose
    // [startDate, endDate] covers today's month, add one Income/Expense for
    // this month unless one with the same recurringId already exists in it.
    // Returns how many transactions were added.
    int generateDueTransactions(FinanceData& data, const Date& today);

    // Menu 4: add / list / edit / delete recurring items.
    void showMenu(FinanceData& data);
}

#endif
