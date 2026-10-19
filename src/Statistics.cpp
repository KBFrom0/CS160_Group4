#include "Statistics.h"

#include "InputHelper.h"
#include "MasterData.h"
#include "AnnualStatistics.h"

namespace Statistics {

PeriodSummary summarize(const FinanceData& data, const Date& from, const Date& to,
                        const std::string& walletId) {
    // TODO
    (void)data; (void)from; (void)to; (void)walletId;
    return PeriodSummary();
}

void showMenu(const FinanceData& data) {
    // TODO
    (void)data;
    InputHelper::showNotImplemented("Statistics & reports");
}

}
