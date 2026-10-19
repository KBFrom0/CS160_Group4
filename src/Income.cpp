#include "Income.h"

Income::Income() : id(""), date(), sourceId(""), amount(0), walletId(""),
                   description(""), recurringId("") {}

Income::Income(const std::string& id, const Date& date, const std::string& sourceId,
               long long amount, const std::string& walletId,
               const std::string& description, const std::string& recurringId)
    : id(id), date(date), sourceId(sourceId), amount(amount), walletId(walletId),
      description(description), recurringId(recurringId) {}
