#ifndef ANNUAL_STATISTICS_H
#define ANNUAL_STATISTICS_H

#include "DynamicArray.h"
#include "FinanceData.h"

// Year-based statistics. The user may select one or several years.
// Owner: Khoa
namespace AnnualStatistics {
    // Asks for years like "2025 2026". Returns false if nothing valid was entered.
    bool inputYears(DynamicArray<int>& years);
    bool containsYear(const DynamicArray<int>& years, int year);

    // Total income, total expense and net balance for the selected years.
    void showOverview(const FinanceData& data);
    // Total income + amount per income source for the selected years.
    void showIncomeBySource(const FinanceData& data);
    // Total expense + amount per expense category for the selected years.
    void showExpenseByCategory(const FinanceData& data);
}

#endif
