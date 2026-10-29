#include "DemoData.h"

namespace DemoData {

void fill(FinanceData& data) {
    data = FinanceData();

    data.categories.add(ExpenseCategory("CAT001", "Food"));
    data.categories.add(ExpenseCategory("CAT002", "Rent"));
    data.categories.add(ExpenseCategory("CAT003", "Transportation"));
    data.categories.add(ExpenseCategory("CAT004", "Shopping"));
    data.nextCategoryNo = 5;

    data.sources.add(IncomeSource("SRC001", "Salary"));
    data.sources.add(IncomeSource("SRC002", "Bonus"));
    data.sources.add(IncomeSource("SRC003", "Savings Interest"));
    data.nextSourceNo = 4;

    data.wallets.add(Wallet("WAL001", "Cash"));
    data.wallets.add(Wallet("WAL002", "Bank Account"));
    data.wallets.add(Wallet("WAL003", "Credit Card"));
    data.nextWalletNo = 4;

    data.incomes.add(Income("INC0001", Date(1, 11, 2025), "SRC001", 14000000, "WAL002", "November salary"));
    data.incomes.add(Income("INC0002", Date(1, 12, 2025), "SRC001", 14000000, "WAL002", "December salary"));
    data.incomes.add(Income("INC0003", Date(20, 12, 2025), "SRC002", 5000000, "WAL002", "Year-end bonus"));
    data.incomes.add(Income("INC0004", Date(31, 12, 2025), "SRC003", 350000, "WAL002", "Interest 2025"));
    data.incomes.add(Income("INC0005", Date(1, 9, 2026), "SRC001", 15000000, "WAL002", "September salary"));
    data.incomes.add(Income("INC0006", Date(1, 10, 2026), "SRC001", 15000000, "WAL002", "October salary"));
    data.incomes.add(Income("INC0007", Date(15, 10, 2026), "SRC002", 2000000, "WAL001", "Project bonus"));
    data.nextIncomeNo = 8;

    data.expenses.add(Expense("EXP0001", Date(5, 11, 2025), "CAT002", 4000000, "WAL002", "Rent November"));
    data.expenses.add(Expense("EXP0002", Date(12, 11, 2025), "CAT001", 850000, "WAL001", "Groceries"));
    data.expenses.add(Expense("EXP0003", Date(5, 12, 2025), "CAT002", 4000000, "WAL002", "Rent December"));
    data.expenses.add(Expense("EXP0004", Date(24, 12, 2025), "CAT004", 2300000, "WAL003", "Christmas gifts"));
    data.expenses.add(Expense("EXP0005", Date(5, 9, 2026), "CAT002", 4000000, "WAL002", "Rent September"));
    data.expenses.add(Expense("EXP0006", Date(18, 9, 2026), "CAT003", 300000, "WAL001", "Grab"));
    data.expenses.add(Expense("EXP0007", Date(5, 10, 2026), "CAT002", 4000000, "WAL002", "Rent October"));
    data.expenses.add(Expense("EXP0008", Date(10, 10, 2026), "CAT001", 120000, "WAL001", "Lunch"));
    data.expenses.add(Expense("EXP0009", Date(22, 10, 2026), "CAT004", 1200000, "WAL003", "New shoes"));
    data.nextExpenseNo = 10;

    data.recurrings.add(RecurringTransaction("REC001", TransactionType::INCOME, "SRC001", 15000000,
                                             "WAL002", "Monthly salary", Date(1, 1, 2025), false, Date()));
    data.recurrings.add(RecurringTransaction("REC002", TransactionType::EXPENSE, "CAT002", 4000000,
                                             "WAL002", "Monthly rent", Date(5, 1, 2025), true, Date(31, 12, 2026)));
    data.nextRecurringNo = 3;
}

}
