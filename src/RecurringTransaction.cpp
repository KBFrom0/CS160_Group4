#include "RecurringTransaction.h"

RecurringTransaction::RecurringTransaction()
    : id(""), type(TransactionType::EXPENSE), refId(""), amount(0), walletId(""),
      description(""), startDate(), hasEndDate(false), endDate() {}

RecurringTransaction::RecurringTransaction(const std::string& id, TransactionType type,
                                           const std::string& refId, long long amount,
                                           const std::string& walletId,
                                           const std::string& description,
                                           const Date& startDate, bool hasEndDate,
                                           const Date& endDate)
    : id(id), type(type), refId(refId), amount(amount), walletId(walletId),
      description(description), startDate(startDate), hasEndDate(hasEndDate),
      endDate(endDate) {}
