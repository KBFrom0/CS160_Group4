#include "Transactions.h"

namespace Transactions {

// ===================== Logic (TODO: Kien) =====================

std::string addIncomeRecord(FinanceData& data, const Date& date, const std::string& sourceId,
                            long long amount, const std::string& walletId,
                            const std::string& description) {
    // 1. Validate basic input
    if (!date.isValid() || amount <= 0 ||
        sourceId.empty() || walletId.empty()) {
        return "";
    }

    // 2. Check whether the income source exists
    bool sourceFound = false;
    for (int i = 0; i < data.sources.size(); i++) {
        if (data.sources[i].getId() == sourceId) {
            sourceFound = true;
            break;
        }
    }

    if (!sourceFound) {
        return "";
    }

    // 3. Check whether the wallet exists
    bool walletFound = false;
    for (int i = 0; i < data.wallets.size(); i++) {
        if (data.wallets[i].getId() == walletId) {
            walletFound = true;
            break;
        }
    }

    if (!walletFound) {
        return "";
    }

    // 4. Generate a new ID
    std::string id = FinanceData::makeId(
        "INC", data.nextIncomeNo, 4
    );

    // 5. Create and store the income
    Income income(id, date, sourceId, amount, walletId, description);
    data.incomes.add(income);

    // 6. Advance the ID counter after adding successfully
    data.nextIncomeNo++;

    return id;
}

std::string addExpenseRecord(FinanceData& data, const Date& date, const std::string& categoryId,
                             long long amount, const std::string& walletId,
                             const std::string& description) {
    
    // 1. Validate basic input
    if (!date.isValid() || amount <= 0 ||
        categoryId.empty() || walletId.empty()) {
        return "";
    }

    // 2. Check whether the expense category exists
    bool categoryFound = false;
    for (int i = 0; i < data.categories.size(); i++) {
        if (data.categories[i].getId() == categoryId) {
            categoryFound = true;
            break;
        }
    }

    if (!categoryFound) {
        return "";
    }

    // 3. Check whether the wallet exists
    bool walletFound = false;
    for (int i = 0; i < data.wallets.size(); i++) {
        if (data.wallets[i].getId() == walletId) {
            walletFound = true;
            break;
        }
    }

    if (!walletFound) {
        return "";
    }

    // 4. Generate a new expense ID
    std::string id = FinanceData::makeId(
        "EXP", data.nextExpenseNo, 4
    );

    // 5. Create and store the expense
    Expense expense(id, date, categoryId, amount, walletId, description);
    data.expenses.add(expense);

    // 6. Advance the ID counter
    data.nextExpenseNo++;

    return id;

}

Date getTransactionDate(const FinanceData& data, const TransactionRef& ref) {
    if (ref.isIncome) {
        return data.incomes[ref.index].getDate();
    }

    return data.expenses[ref.index].getDate();
}

bool isOlder(const FinanceData& data,
             const TransactionRef& a,
             const TransactionRef& b) {
    return getTransactionDate(data, a) < getTransactionDate(data, b);
}

DynamicArray<TransactionRef> getSortedTransactions(const FinanceData& data) {
    DynamicArray<TransactionRef> transactions;

    // Step 1: Add all income references.
    for (int i = 0; i < data.incomes.size(); i++) {
        TransactionRef ref;
        ref.isIncome = true;
        ref.index = i;

        transactions.add(ref);
    }

    // Step 2: Add all expense references.
    for (int i = 0; i < data.expenses.size(); i++) {
        TransactionRef ref;
        ref.isIncome = false;
        ref.index = i;

        transactions.add(ref);
    }

    // Step 3: Sort transactions by date (newest first).
    for (int i = 1; i < transactions.size(); i++) {
        TransactionRef key = transactions[i];
        int j = i - 1;

        while (j >= 0 && isOlder(data, transactions[j], key)) {
            transactions[j + 1] = transactions[j];
            j--;
        }

        transactions[j + 1] = key;
    }

    return transactions;
}

bool updateIncome(FinanceData& data, int index, const Income& updated) {
    // TODO
    (void)data; (void)index; (void)updated;
    return false;
}

bool updateExpense(FinanceData& data, int index, const Expense& updated) {
    // TODO
    (void)data; (void)index; (void)updated;
    return false;
}

bool deleteTransaction(FinanceData& data, const TransactionRef& ref) {
    // TODO
    (void)data; (void)ref;
    return false;
}

}
