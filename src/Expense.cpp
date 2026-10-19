#include "Expense.h"

Expense::Expense() : id(""), date(), categoryId(""), amount(0), walletId(""),
                     description(""), recurringId("") {}

Expense::Expense(const std::string& id, const Date& date, const std::string& categoryId,
                 long long amount, const std::string& walletId,
                 const std::string& description, const std::string& recurringId)
    : id(id), date(date), categoryId(categoryId), amount(amount), walletId(walletId),
      description(description), recurringId(recurringId) {}
