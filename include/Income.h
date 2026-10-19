#ifndef INCOME_H
#define INCOME_H

#include <string>
#include "Date.h"

// One income transaction. Stores IDs only; the UI looks up names to display.
class Income {
private:
    std::string id;           // e.g. "INC0001"
    Date date;
    std::string sourceId;     // -> IncomeSource
    long long amount;         // whole VND, always > 0
    std::string walletId;     // -> Wallet
    std::string description;
    std::string recurringId;  // empty if entered manually

public:
    Income();
    Income(const std::string& id, const Date& date, const std::string& sourceId,
           long long amount, const std::string& walletId,
           const std::string& description, const std::string& recurringId = "");

    std::string getId() const { return id; }
    Date getDate() const { return date; }
    std::string getSourceId() const { return sourceId; }
    long long getAmount() const { return amount; }
    std::string getWalletId() const { return walletId; }
    std::string getDescription() const { return description; }
    std::string getRecurringId() const { return recurringId; }

    void setDate(const Date& value) { date = value; }
    void setSourceId(const std::string& value) { sourceId = value; }
    void setAmount(long long value) { amount = value; }
    void setWalletId(const std::string& value) { walletId = value; }
    void setDescription(const std::string& value) { description = value; }
};

#endif
