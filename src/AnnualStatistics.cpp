#include "AnnualStatistics.h"

namespace AnnualStatistics {

// ===================== Logic (TODO: Khoa) =====================

bool containsYear(const DynamicArray<int>& years, int year) {
    for (int i = 0; i < years.size(); i++) {
        if (years[i] == year) {
            return true;
        }
    }
    return false;
}

Statistics::PeriodSummary summarizeYears(const FinanceData& data, const DynamicArray<int>& years) {
    // TODO
    (void)data; (void)years;
    return Statistics::PeriodSummary();
}

DynamicArray<long long> incomeBySource(const FinanceData& data, const DynamicArray<int>& years) {
    // TODO
    (void)data; (void)years;
    return DynamicArray<long long>();
}

DynamicArray<long long> expenseByCategory(const FinanceData& data, const DynamicArray<int>& years) {
    // TODO
    (void)data; (void)years;
    return DynamicArray<long long>();
}

}
