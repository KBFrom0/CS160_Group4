#ifndef STATISTICS_H
#define STATISTICS_H

#include <string>
#include "Date.h"
#include "FinanceData.h"

// Time-range statistics + the Statistics menu (menu 6).
// Owner: Quan
namespace Statistics {
    struct PeriodSummary {
        long long totalIncome = 0;
        long long totalExpense = 0;
        long long net() const { return totalIncome - totalExpense; }
    };

    // Totals for transactions with from <= date <= to.
    // walletId = "" means all wallets; otherwise only that wallet.
    PeriodSummary summarize(const FinanceData& data, const Date& from, const Date& to,
                            const std::string& walletId = "");

    // Menu 6. Options:
    //   1. Income / expense in a date range          (Quan)
    //   2. Income / expense in a date range by wallet (Quan)
    //   3. Annual overview          -> AnnualStatistics::showOverview
    //   4. Annual income by source  -> AnnualStatistics::showIncomeBySource
    //   5. Annual expense by category -> AnnualStatistics::showExpenseByCategory
    void showMenu(const FinanceData& data);
}

#endif
