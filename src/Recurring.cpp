#include "Recurring.h"

#include "InputHelper.h"
#include "MasterData.h"

namespace Recurring {

int generateDueTransactions(FinanceData& data, const Date& today) {
    // TODO
    (void)data; (void)today;
    return 0;
}

void showMenu(FinanceData& data) {
    // TODO
    (void)data;
    InputHelper::showNotImplemented("Recurring transactions");
}

}
