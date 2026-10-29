#include "Transactions.h"

namespace Transactions {

// ===================== Logic (TODO: Kien) =====================

std::string addIncomeRecord(FinanceData& data, const Date& date, const std::string& sourceId,
                            long long amount, const std::string& walletId,
                            const std::string& description) {
    // TODO
    (void)data; (void)date; (void)sourceId; (void)amount; (void)walletId; (void)description;
    return "";
}

std::string addExpenseRecord(FinanceData& data, const Date& date, const std::string& categoryId,
                             long long amount, const std::string& walletId,
                             const std::string& description) {
    // TODO
    (void)data; (void)date; (void)categoryId; (void)amount; (void)walletId; (void)description;
    return "";
}

DynamicArray<TransactionRef> getSortedTransactions(const FinanceData& data) {
    // TODO
    (void)data;
    return DynamicArray<TransactionRef>();
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
