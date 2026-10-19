#ifndef TRANSACTIONS_H
#define TRANSACTIONS_H

#include "FinanceData.h"

// Income / expense transactions: add, list, edit, delete.
// Owner: Kien
namespace Transactions {
    // Menu 1 / 2. Source, category and wallet are picked with
    // MasterData::chooseXxx(); store the ID, display the name.
    void addIncome(FinanceData& data);
    void addExpense(FinanceData& data);

    // Menu 3: list all transactions (sorted by date) and edit / delete one.
    void showMenu(FinanceData& data);
}

#endif
