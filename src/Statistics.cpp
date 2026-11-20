#include "Statistics.h"

namespace Statistics {

// ===================== Logic (TODO: Quan) =====================

PeriodSummary summarize(const FinanceData& data, const Date& from, const Date& to,
                        const std::string& walletId) {
    PeriodSummary result;

    for (int i = 0; i < data.incomes.size(); i++) {
        const Income& inc = data.incomes.get(i);
        if (!inc.getDate().isBetween(from, to))
            continue;
        if (!walletId.empty() && inc.getWalletId() != walletId)
            continue;
        result.totalIncome += inc.getAmount();
    }

    for (int i = 0; i < data.expenses.size(); i++) {
        const Expense& exp = data.expenses.get(i);
        if (!exp.getDate().isBetween(from, to))
            continue;
        if (!walletId.empty() && exp.getWalletId() != walletId)
            continue;
        result.totalExpense += exp.getAmount();
    }

    return result;
}

}
