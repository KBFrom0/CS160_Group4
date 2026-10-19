#ifndef RECURRING_TRANSACTION_H
#define RECURRING_TRANSACTION_H

#include <string>
#include "Date.h"

enum class TransactionType { INCOME = 0, EXPENSE = 1 };

// A monthly template. On app start, one real Income/Expense is generated for
// the current month if [startDate, endDate] covers it and it was not
// generated yet (generated transactions carry this object's id).
class RecurringTransaction {
private:
    std::string id;           // e.g. "REC001"
    TransactionType type;
    std::string refId;        // IncomeSource id if INCOME, ExpenseCategory id if EXPENSE
    long long amount;
    std::string walletId;
    std::string description;
    Date startDate;           // required
    bool hasEndDate;          // false = recurs indefinitely
    Date endDate;             // only meaningful when hasEndDate is true

public:
    RecurringTransaction();
    RecurringTransaction(const std::string& id, TransactionType type,
                         const std::string& refId, long long amount,
                         const std::string& walletId, const std::string& description,
                         const Date& startDate, bool hasEndDate, const Date& endDate);

    std::string getId() const { return id; }
    TransactionType getType() const { return type; }
    std::string getRefId() const { return refId; }
    long long getAmount() const { return amount; }
    std::string getWalletId() const { return walletId; }
    std::string getDescription() const { return description; }
    Date getStartDate() const { return startDate; }
    bool getHasEndDate() const { return hasEndDate; }
    Date getEndDate() const { return endDate; }

    void setRefId(const std::string& value) { refId = value; }
    void setAmount(long long value) { amount = value; }
    void setWalletId(const std::string& value) { walletId = value; }
    void setDescription(const std::string& value) { description = value; }
    void setStartDate(const Date& value) { startDate = value; }
    void setEndDate(bool has, const Date& value) { hasEndDate = has; endDate = value; }
};

#endif
