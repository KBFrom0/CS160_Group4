#ifndef TRANSACTIONS_H
#define TRANSACTIONS_H

#include <string>
#include "DynamicArray.h"
#include "FinanceData.h"

// Income / expense transactions.
// Logic marked "TODO" in Transactions.cpp: Kien. The GUI (src/gui) calls these.
namespace Transactions {
    // Points to one transaction: data.incomes[index] or data.expenses[index].
    struct TransactionRef {
        bool isIncome = true;
        int index = -1;
    };

    // ===== Logic (TODO: Kien) =====

    // Create the transaction with a new ID (INC0001 / EXP0001) and add it to data.
    // Return the new ID, or "" on failure.
    std::string addIncomeRecord(FinanceData& data, const Date& date, const std::string& sourceId,
                                long long amount, const std::string& walletId,
                                const std::string& description);
    std::string addExpenseRecord(FinanceData& data, const Date& date, const std::string& categoryId,
                                 long long amount, const std::string& walletId,
                                 const std::string& description);

    // All incomes and expenses, sorted by date (newest first).
    DynamicArray<TransactionRef> getSortedTransactions(const FinanceData& data);

    // Replace data.incomes[index] / data.expenses[index]. Return false on failure.
    bool updateIncome(FinanceData& data, int index, const Income& updated);
    bool updateExpense(FinanceData& data, int index, const Expense& updated);
    bool deleteTransaction(FinanceData& data, const TransactionRef& ref);
}

#endif
