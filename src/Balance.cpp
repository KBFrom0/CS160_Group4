#include "Balance.h"

namespace Balance {

long long walletBalance(const FinanceData& data, const std::string& walletId) {
    long long balance = 0;

    // Income adds money to the wallet.
    for (int i = 0; i < data.incomes.size(); i++) {
        if (data.incomes[i].getWalletId() == walletId) {
            balance += data.incomes[i].getAmount();
        }
    }

    // Expense takes money out of the wallet (the balance may become negative).
    for (int i = 0; i < data.expenses.size(); i++) {
        if (data.expenses[i].getWalletId() == walletId) {
            balance -= data.expenses[i].getAmount();
        }
    }

    return balance;
}

long long totalBalance(const FinanceData& data) {
    long long total = 0;
    for (int i = 0; i < data.wallets.size(); i++) {
        total += walletBalance(data, data.wallets[i].getId());
    }
    return total;
}

}
