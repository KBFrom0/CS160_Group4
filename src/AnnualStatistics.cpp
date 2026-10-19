#include "AnnualStatistics.h"

#include "InputHelper.h"
#include "MasterData.h"

namespace AnnualStatistics {

bool inputYears(DynamicArray<int>& years) {
    // TODO
    (void)years;
    return false;
}

bool containsYear(const DynamicArray<int>& years, int year) {
    // TODO
    (void)years; (void)year;
    return false;
}

void showOverview(const FinanceData& data) {
    // TODO
    (void)data;
    InputHelper::showNotImplemented("Annual overview");
}

void showIncomeBySource(const FinanceData& data) {
    // TODO
    (void)data;
    InputHelper::showNotImplemented("Annual income by source");
}

void showExpenseByCategory(const FinanceData& data) {
    // TODO
    (void)data;
    InputHelper::showNotImplemented("Annual expense by category");
}

}
