#ifndef FINANCE_DATA_H
#define FINANCE_DATA_H

#include <string>
#include "DynamicArray.h"
#include "ExpenseCategory.h"
#include "IncomeSource.h"
#include "Wallet.h"
#include "Income.h"
#include "Expense.h"
#include "RecurringTransaction.h"

// All application data in one place. Every module receives a reference to
// this object; Storage saves/loads it as a whole.
struct FinanceData {
    DynamicArray<ExpenseCategory> categories;
    DynamicArray<IncomeSource> sources;
    DynamicArray<Wallet> wallets;
    DynamicArray<Income> incomes;
    DynamicArray<Expense> expenses;
    DynamicArray<RecurringTransaction> recurrings;

    // Next number used when generating an ID (persisted, never reused).
    int nextCategoryNo = 1;
    int nextSourceNo = 1;
    int nextWalletNo = 1;
    int nextIncomeNo = 1;
    int nextExpenseNo = 1;
    int nextRecurringNo = 1;

    // makeId("CAT", 7, 3) -> "CAT007"
    static std::string makeId(const std::string& prefix, int number, int width);
};

#endif
