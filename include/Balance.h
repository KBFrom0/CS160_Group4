#ifndef BALANCE_H
#define BALANCE_H

#include <string>
#include "FinanceData.h"

// Wallet balances, computed from transactions (balance = income - expense).
// Owner: Duyen
namespace Balance {
    long long walletBalance(const FinanceData& data, const std::string& walletId);
    long long totalBalance(const FinanceData& data);  // sum over all wallets
}

#endif
