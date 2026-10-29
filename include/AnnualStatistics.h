#ifndef ANNUAL_STATISTICS_H
#define ANNUAL_STATISTICS_H

#include "DynamicArray.h"
#include "FinanceData.h"
#include "Statistics.h"

// Year-based statistics. The user may select one or several years.
// Logic marked "TODO" in AnnualStatistics.cpp: Khoa. The GUI (src/gui) calls these.
namespace AnnualStatistics {
    // ===== Logic (TODO: Khoa) =====

    bool containsYear(const DynamicArray<int>& years, int year);

    // Totals of all transactions whose year is in `years`.
    Statistics::PeriodSummary summarizeYears(const FinanceData& data, const DynamicArray<int>& years);

    // Amount per item for the selected years.
    // result[i] belongs to data.sources[i] / data.categories[i] (same size as that list).
    DynamicArray<long long> incomeBySource(const FinanceData& data, const DynamicArray<int>& years);
    DynamicArray<long long> expenseByCategory(const FinanceData& data, const DynamicArray<int>& years);
}

#endif
