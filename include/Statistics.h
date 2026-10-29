#ifndef STATISTICS_H
#define STATISTICS_H

#include <string>
#include "Date.h"
#include "FinanceData.h"

// Time-range statistics.
// Logic marked "TODO" in Statistics.cpp: Quan. The GUI (src/gui) calls these.
namespace Statistics {
    struct PeriodSummary {
        long long totalIncome = 0;
        long long totalExpense = 0;
        long long net() const { return totalIncome - totalExpense; }
    };

    // ===== Logic (TODO: Quan) =====

    // Totals for transactions with from <= date <= to.
    // walletId = "" means all wallets; otherwise only that wallet.
    PeriodSummary summarize(const FinanceData& data, const Date& from, const Date& to,
                            const std::string& walletId = "");
}

#endif
