#include "AnnualStatistics.h"

#include "MasterData.h"

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
    Statistics::PeriodSummary summary;
    for (int i = 0; i < data.incomes.size(); i++) {
        if (containsYear(years, data.incomes[i].getDate().getYear())) {
            summary.totalIncome += data.incomes[i].getAmount();
        }
    }
    for (int i = 0; i < data.expenses.size(); i++) {
        if (containsYear(years, data.expenses[i].getDate().getYear())) {
            summary.totalExpense += data.expenses[i].getAmount();
        }
    }
    return summary;
}

DynamicArray<long long> incomeBySource(const FinanceData& data, const DynamicArray<int>& years) {
    // One slot per source, so result[i] matches data.sources[i] even when it is 0.
    DynamicArray<long long> result;
    for (int i = 0; i < data.sources.size(); i++) {
        result.add(0);
    }

    for (int i = 0; i < data.incomes.size(); i++) {
        const Income& income = data.incomes[i];
        if (!containsYear(years, income.getDate().getYear())) {
            continue;
        }
        int index = MasterData::findSourceIndex(data, income.getSourceId());
        if (index != -1) {
            result[index] += income.getAmount();
        }
    }
    return result;
}

DynamicArray<long long> expenseByCategory(const FinanceData& data, const DynamicArray<int>& years) {
    // One slot per category, so result[i] matches data.categories[i] even when it is 0.
    DynamicArray<long long> result;
    for (int i = 0; i < data.categories.size(); i++) {
        result.add(0);
    }

    for (int i = 0; i < data.expenses.size(); i++) {
        const Expense& expense = data.expenses[i];
        if (!containsYear(years, expense.getDate().getYear())) {
            continue;
        }
        int index = MasterData::findCategoryIndex(data, expense.getCategoryId());
        if (index != -1) {
            result[index] += expense.getAmount();
        }
    }
    return result;
}

}
