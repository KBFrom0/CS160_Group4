// Unit tests for the core building blocks (DynamicArray, Date, FinanceData::makeId).
// Build & run: see README.md. Prints FAIL lines and returns 1 if anything breaks.
#include "Transactions.h"
#include <iostream>
#include <string>
#include "DynamicArray.h"
#include "Date.h"
#include "FinanceData.h"
#include "Format.h"
#include "Balance.h"
#include "MasterData.h"
#include "Recurring.h"
#include "AnnualStatistics.h"
#include "Storage.h"
#include "Auth.h"
#include <cstdio>
#include <fstream>

static int failures = 0;

#define CHECK(cond)                                                        \
    do {                                                                   \
        if (!(cond)) {                                                     \
            std::cout << "FAIL " << __FILE__ << ":" << __LINE__ << "  "   \
                      << #cond << '\n';                                    \
            failures++;                                                    \
        }                                                                  \
    } while (0)

static void testDynamicArray() {
    DynamicArray<int> a;
    CHECK(a.size() == 0);
    CHECK(a.isEmpty());

    for (int i = 0; i < 100; i++) a.add(i);
    CHECK(a.size() == 100);
    CHECK(a[0] == 0 && a[99] == 99);

    a.removeAt(0);
    CHECK(a.size() == 99);
    CHECK(a[0] == 1);
    a.removeAt(a.size() - 1);
    CHECK(a[a.size() - 1] == 98);

    // Copies must be independent (deep copy).
    DynamicArray<int> b = a;
    b.set(0, -1);
    CHECK(a[0] == 1);
    CHECK(b[0] == -1);

    DynamicArray<int> c;
    c = a;
    c.add(500);
    CHECK(c.size() == a.size() + 1);

    c = c;  // self-assignment must be safe
    CHECK(c.size() == a.size() + 1);

    bool threw = false;
    try { a.get(1000); } catch (const std::out_of_range&) { threw = true; }
    CHECK(threw);

    DynamicArray<std::string> s;
    s.add("Food");
    s.add("Rent");
    CHECK(s[1] == "Rent");
    s.clear();
    CHECK(s.isEmpty());
}

static void testDate() {
    Date d;
    CHECK(Date::parse("29/02/2024", d));      // leap year
    CHECK(!Date::parse("29/02/2023", d));     // not leap
    CHECK(!Date::parse("31/04/2025", d));
    CHECK(!Date::parse("1-1-2025", d));
    CHECK(!Date::parse("01/01/2025x", d));
    CHECK(Date::parse("5/3/2025", d));
    CHECK(d.toString() == "05/03/2025");

    Date a(1, 1, 2025), b(31, 12, 2025), mid(15, 6, 2025);
    CHECK(a < b);
    CHECK(mid.isBetween(a, b));
    CHECK(a.isBetween(a, b));
    CHECK(!Date(1, 1, 2026).isBetween(a, b));
    CHECK(Date::today().isValid());
}

static void testHelpers() {
    CHECK(FinanceData::makeId("CAT", 7, 3) == "CAT007");
    CHECK(FinanceData::makeId("INC", 12345, 4) == "INC12345");
    CHECK(Format::money(1500000) == "1,500,000");
    CHECK(Format::money(-999) == "-999");
    CHECK(Format::money(0) == "0");
}

static void testMasterDataLookup() {
    FinanceData data;

    // Empty lists: nothing can be found.
    CHECK(MasterData::findCategoryIndex(data, "CAT001") == -1);
    CHECK(MasterData::getWalletName(data, "WAL001") == "(unknown)");

    data.categories.add(ExpenseCategory("CAT001", "Food"));
    data.categories.add(ExpenseCategory("CAT002", "Rent"));
    data.sources.add(IncomeSource("SRC001", "Salary"));
    data.sources.add(IncomeSource("SRC002", "Bonus"));
    data.sources.add(IncomeSource("SRC003", "Savings Interest"));
    data.wallets.add(Wallet("WAL001", "Cash"));
    data.wallets.add(Wallet("WAL002", "Bank Account"));

    // Find the first, middle and last item in each list.
    CHECK(MasterData::findCategoryIndex(data, "CAT001") == 0);
    CHECK(MasterData::findCategoryIndex(data, "CAT002") == 1);

    CHECK(MasterData::findSourceIndex(data, "SRC001") == 0);
    CHECK(MasterData::findSourceIndex(data, "SRC002") == 1);
    CHECK(MasterData::findSourceIndex(data, "SRC003") == 2);

    CHECK(MasterData::findWalletIndex(data, "WAL001") == 0);
    CHECK(MasterData::findWalletIndex(data, "WAL002") == 1);

    // Not found: unknown id, empty id, wrong case, id of another list.
    CHECK(MasterData::findCategoryIndex(data, "CAT999") == -1);
    CHECK(MasterData::findSourceIndex(data, "") == -1);
    CHECK(MasterData::findWalletIndex(data, "wal001") == -1);
    CHECK(MasterData::findWalletIndex(data, "SRC001") == -1);

    CHECK(MasterData::getCategoryName(data, "CAT002") == "Rent");
    CHECK(MasterData::getSourceName(data, "SRC003") == "Savings Interest");
    CHECK(MasterData::getWalletName(data, "WAL001") == "Cash");
    CHECK(MasterData::getCategoryName(data, "CAT999") == "(unknown)");
    CHECK(MasterData::getSourceName(data, "") == "(unknown)");
    CHECK(MasterData::getWalletName(data, "CAT001") == "(unknown)");

    // The name follows a rename (it is looked up, not copied).
    data.wallets[0].setName("Wallet");
    CHECK(MasterData::getWalletName(data, "WAL001") == "Wallet");
}

static void testMasterDataAddItem() {
    FinanceData data;
    std::string message;

    // A valid name creates an item with a generated ID and no message.
    CHECK(MasterData::addItem(data, MasterData::Kind::CATEGORY, "Food", message));
    CHECK(message == "");
    CHECK(MasterData::addItem(data, MasterData::Kind::SOURCE, "Salary", message));
    CHECK(MasterData::addItem(data, MasterData::Kind::WALLET, "Cash", message));
    CHECK(MasterData::addItem(data, MasterData::Kind::WALLET, "Bank Account", message));
    CHECK(data.categories.size() == 1 && data.sources.size() == 1 && data.wallets.size() == 2);
    CHECK(data.categories[0].getId() == "CAT001");
    CHECK(data.sources[0].getId() == "SRC001");
    CHECK(data.wallets[0].getId() == "WAL001");
    CHECK(data.wallets[1].getId() == "WAL002");
    CHECK(data.wallets[1].getName() == "Bank Account");
    CHECK(data.nextWalletNo == 3);

    // Empty name, or only spaces: rejected with a reason, nothing changes.
    CHECK(!MasterData::addItem(data, MasterData::Kind::CATEGORY, "", message));
    CHECK(message != "");
    message = "";
    CHECK(!MasterData::addItem(data, MasterData::Kind::CATEGORY, "   ", message));
    CHECK(message != "");
    CHECK(data.categories.size() == 1);
    CHECK(data.nextCategoryNo == 2);

    // Duplicate name in the same list: rejected (also with other case or extra spaces).
    message = "";
    CHECK(!MasterData::addItem(data, MasterData::Kind::WALLET, "Cash", message));
    CHECK(message != "");
    CHECK(!MasterData::addItem(data, MasterData::Kind::WALLET, "cash", message));
    CHECK(!MasterData::addItem(data, MasterData::Kind::WALLET, "  CASH  ", message));
    CHECK(data.wallets.size() == 2);
    CHECK(data.nextWalletNo == 3);

    // The same name in another list is fine.
    CHECK(MasterData::addItem(data, MasterData::Kind::CATEGORY, "Cash", message));
    CHECK(data.categories[1].getId() == "CAT002");

    // Spaces around the name are removed before saving.
    CHECK(MasterData::addItem(data, MasterData::Kind::SOURCE, "  Bonus ", message));
    CHECK(data.sources[1].getName() == "Bonus");

    // The ID comes from the counter, so a gap (deleted item) is not reused.
    data.nextSourceNo = 10;
    CHECK(MasterData::addItem(data, MasterData::Kind::SOURCE, "Gift", message));
    CHECK(data.sources[2].getId() == "SRC010");
}

static void testBalance() {
    FinanceData data;

    // No wallets: total is 0, and an unknown wallet has balance 0.
    CHECK(Balance::totalBalance(data) == 0);
    CHECK(Balance::walletBalance(data, "WAL001") == 0);

    data.wallets.add(Wallet("WAL001", "Cash"));
    data.wallets.add(Wallet("WAL002", "Bank Account"));
    data.wallets.add(Wallet("WAL003", "Credit Card"));

    // A wallet without transactions has balance 0.
    CHECK(Balance::walletBalance(data, "WAL001") == 0);

    // Balance = income - expense, counted per wallet.
    data.incomes.add(Income("INC0001", Date(1, 1, 2026), "SRC001", 1000000, "WAL001", ""));
    data.incomes.add(Income("INC0002", Date(2, 1, 2026), "SRC001", 500000, "WAL002", ""));
    data.expenses.add(Expense("EXP0001", Date(3, 1, 2026), "CAT001", 300000, "WAL001", ""));
    CHECK(Balance::walletBalance(data, "WAL001") == 700000);
    CHECK(Balance::walletBalance(data, "WAL002") == 500000);
    CHECK(Balance::walletBalance(data, "WAL003") == 0);
    CHECK(Balance::totalBalance(data) == 1200000);

    // A wallet can go negative (spent more than it has).
    data.expenses.add(Expense("EXP0002", Date(4, 1, 2026), "CAT001", 2000000, "WAL003", ""));
    CHECK(Balance::walletBalance(data, "WAL003") == -2000000);
    CHECK(Balance::totalBalance(data) == -800000);

    // Big amounts must not overflow (long long, not int).
    data.incomes.add(Income("INC0003", Date(5, 1, 2026), "SRC001", 5000000000LL, "WAL002", ""));
    CHECK(Balance::walletBalance(data, "WAL002") == 5000500000LL);

    // The balance follows the transactions: removing one changes it.
    data.expenses.removeAt(0);
    CHECK(Balance::walletBalance(data, "WAL001") == 1000000);

    // Transactions of a wallet that does not exist are not part of the total.
    data.incomes.add(Income("INC0004", Date(6, 1, 2026), "SRC001", 123456, "WAL999", ""));
    CHECK(Balance::walletBalance(data, "WAL999") == 123456);
    CHECK(Balance::totalBalance(data) == 1000000 + 5000500000LL - 2000000);

}

static void testSortedTransactions() {
    FinanceData data;

    // Add income transactions.
    data.incomes.add(Income(
        "INC001", Date(1, 1, 2025), "SRC001",
        100000, "WAL001", "Income 1"
    ));

    data.incomes.add(Income(
        "INC002", Date(3, 1, 2025), "SRC001",
        200000, "WAL001", "Income 2"
    ));

    data.incomes.add(Income(
        "INC003", Date(3, 1, 2025), "SRC001",
        300000, "WAL001", "Income 3"
    ));

    // Add expense transactions.
    data.expenses.add(Expense(
        "EXP001", Date(2, 1, 2025), "CAT001",
        50000, "WAL001", "Expense 1"
    ));

    data.expenses.add(Expense(
        "EXP002", Date(3, 1, 2025), "CAT001",
        60000, "WAL001", "Expense 2"
    ));

    DynamicArray<Transactions::TransactionRef> sorted =
        Transactions::getSortedTransactions(data);

    CHECK(sorted.size() == 5);

    // Newest date must come first.
    CHECK(sorted[0].isIncome && sorted[0].index == 1);
    CHECK(sorted[1].isIncome && sorted[1].index == 2);

    // Same date: preserve original order.
    CHECK(sorted[2].isIncome == false && sorted[2].index == 1);

    // Older dates come afterward.
    CHECK(sorted[3].isIncome == false && sorted[3].index == 0);
    CHECK(sorted[4].isIncome && sorted[4].index == 0);
}

static void testDeleteTransaction() {
    FinanceData data;

    data.incomes.add(Income(
        "INC001", Date(1, 1, 2025), "SRC001",
        100000, "WAL001", "A"
    ));
    data.incomes.add(Income(
        "INC002", Date(2, 1, 2025), "SRC001",
        200000, "WAL001", "B"
    ));
    data.incomes.add(Income(
        "INC003", Date(3, 1, 2025), "SRC001",
        300000, "WAL001", "C"
    ));

    // Delete the first income.
    CHECK(Transactions::deleteTransaction(
        data, {true, 0}
    ));
    CHECK(data.incomes.size() == 2);
    CHECK(data.incomes[0].getId() == "INC002");

    // Delete the last income in the remaining array.
    CHECK(Transactions::deleteTransaction(
        data, {true, 1}
    ));
    CHECK(data.incomes.size() == 1);
    CHECK(data.incomes[0].getId() == "INC002");

    // Delete the remaining income.
    CHECK(Transactions::deleteTransaction(
        data, {true, 0}
    ));
    CHECK(data.incomes.isEmpty());

    // Invalid income index.
    CHECK(!Transactions::deleteTransaction(
        data, {true, 0}
    ));

    // Test expense deletion.
    data.expenses.add(Expense(
        "EXP001", Date(1, 1, 2025), "CAT001",
        10000, "WAL001", "A"
    ));
    data.expenses.add(Expense(
        "EXP002", Date(2, 1, 2025), "CAT001",
        20000, "WAL001", "B"
    ));
    data.expenses.add(Expense(
        "EXP003", Date(3, 1, 2025), "CAT001",
        30000, "WAL001", "C"
    ));

    // Delete the middle expense.
    CHECK(Transactions::deleteTransaction(
        data, {false, 1}
    ));
    CHECK(data.expenses.size() == 2);
    CHECK(data.expenses[0].getId() == "EXP001");
    CHECK(data.expenses[1].getId() == "EXP003");

    // Delete the first and last remaining expenses.
    CHECK(Transactions::deleteTransaction(
        data, {false, 0}
    ));
    CHECK(data.expenses[0].getId() == "EXP003");

    CHECK(Transactions::deleteTransaction(
        data, {false, 0}
    ));
    CHECK(data.expenses.isEmpty());

    // Invalid expense index.
    CHECK(!Transactions::deleteTransaction(
        data, {false, 0}
    ));
}

static void testUpdateTransactions() {
    FinanceData data;

    // Prepare valid sources, categories and wallets.
    data.sources.add(IncomeSource("SRC001", "Salary"));
    data.sources.add(IncomeSource("SRC002", "Bonus"));

    data.categories.add(ExpenseCategory("CAT001", "Food"));
    data.categories.add(ExpenseCategory("CAT002", "Transport"));

    data.wallets.add(Wallet("WAL001", "Cash"));
    data.wallets.add(Wallet("WAL002", "Bank"));

    // Test updating income to another wallet.
    data.incomes.add(Income(
        "INC001", Date(1, 1, 2025), "SRC001",
        100000, "WAL001", "Old income", "REC001"
    ));

    Income updatedIncome(
        "INC001", Date(2, 1, 2025), "SRC002",
        200000, "WAL002", "Updated income", "REC001"
    );

    CHECK(Transactions::updateIncome(data, 0, updatedIncome));

    CHECK(data.incomes[0].getWalletId() == "WAL002");
    CHECK(data.incomes[0].getSourceId() == "SRC002");
    CHECK(data.incomes[0].getAmount() == 200000);
    CHECK(data.incomes[0].getId() == "INC001");
    CHECK(data.incomes[0].getRecurringId() == "REC001");

    // Invalid income index.
    CHECK(!Transactions::updateIncome(data, 1, updatedIncome));

    // Test updating expense to another wallet.
    data.expenses.add(Expense(
        "EXP001", Date(1, 1, 2025), "CAT001",
        50000, "WAL001", "Old expense", "REC002"
    ));

    Expense updatedExpense(
        "EXP001", Date(2, 1, 2025), "CAT002",
        70000, "WAL002", "Updated expense", "REC002"
    );

    CHECK(Transactions::updateExpense(data, 0, updatedExpense));

    CHECK(data.expenses[0].getWalletId() == "WAL002");
    CHECK(data.expenses[0].getCategoryId() == "CAT002");
    CHECK(data.expenses[0].getAmount() == 70000);
    CHECK(data.expenses[0].getId() == "EXP001");
    CHECK(data.expenses[0].getRecurringId() == "REC002");

    // Invalid expense index.
    CHECK(!Transactions::updateExpense(data, 1, updatedExpense));
}

static void testRecurringCrud() {
    FinanceData data;
    data.sources.add(IncomeSource("SRC001", "Salary"));
    data.categories.add(ExpenseCategory("CAT001", "Rent"));
    data.wallets.add(Wallet("WAL001", "Cash"));

    Date start(1, 1, 2026);
    RecurringTransaction salary("ignored", TransactionType::INCOME, "SRC001", 1000, "WAL001", "Pay",
                                start, false, Date());
    RecurringTransaction rent("", TransactionType::EXPENSE, "CAT001", 500, "WAL001", "Rent",
                              start, true, Date(1, 6, 2026));

    CHECK(Recurring::addRecurring(data, salary) == "REC001");
    CHECK(Recurring::addRecurring(data, rent) == "REC002");
    CHECK(data.recurrings.size() == 2);
    CHECK(data.recurrings[0].getId() == "REC001");

    // Rejected input changes nothing and does not use up an ID.
    CHECK(Recurring::addRecurring(data, RecurringTransaction("", TransactionType::INCOME, "SRC001", 0,
                                  "WAL001", "", start, false, Date())) == "");
    CHECK(Recurring::addRecurring(data, RecurringTransaction("", TransactionType::INCOME, "CAT001", 1,
                                  "WAL001", "", start, false, Date())) == "");
    CHECK(Recurring::addRecurring(data, RecurringTransaction("", TransactionType::EXPENSE, "CAT001", 1,
                                  "WAL999", "", start, false, Date())) == "");
    CHECK(Recurring::addRecurring(data, RecurringTransaction("", TransactionType::EXPENSE, "CAT001", 1,
                                  "WAL001", "", start, true, Date(31, 12, 2025))) == "");
    CHECK(data.recurrings.size() == 2);
    CHECK(data.nextRecurringNo == 3);

    // Update keeps the existing ID.
    RecurringTransaction changed("REC999", TransactionType::INCOME, "SRC001", 2000, "WAL001", "Raise",
                                 start, false, Date());
    CHECK(Recurring::updateRecurring(data, 0, changed));
    CHECK(data.recurrings[0].getId() == "REC001");
    CHECK(data.recurrings[0].getAmount() == 2000);
    CHECK(!Recurring::updateRecurring(data, 2, changed));
    CHECK(!Recurring::updateRecurring(data, -1, changed));

    // A deleted ID is not reused.
    CHECK(!Recurring::removeRecurring(data, 2));
    CHECK(!Recurring::removeRecurring(data, -1));
    CHECK(Recurring::removeRecurring(data, 0));
    CHECK(data.recurrings.size() == 1);
    CHECK(data.recurrings[0].getId() == "REC002");
    CHECK(Recurring::addRecurring(data, salary) == "REC003");
}

static void testContainsYear() {
    DynamicArray<int> years;
    CHECK(!AnnualStatistics::containsYear(years, 2026));
    years.add(2025);
    years.add(2026);
    CHECK(AnnualStatistics::containsYear(years, 2026));
    CHECK(!AnnualStatistics::containsYear(years, 2024));
}

static void testGenerateDueTransactions() {
    FinanceData data;
    data.sources.add(IncomeSource("SRC001", "Salary"));
    data.categories.add(ExpenseCategory("CAT001", "Rent"));
    data.wallets.add(Wallet("WAL001", "Cash"));
    data.recurrings.add(RecurringTransaction("REC001", TransactionType::INCOME, "SRC001", 1000,
                                             "WAL001", "Pay", Date(31, 1, 2026), false, Date()));
    data.recurrings.add(RecurringTransaction("REC002", TransactionType::EXPENSE, "CAT001", 500,
                                             "WAL001", "Rent", Date(5, 3, 2026), true, Date(10, 6, 2026)));

    // Before the start month of REC002: only REC001 is due.
    CHECK(Recurring::generateDueTransactions(data, Date(15, 2, 2026)) == 1);
    CHECK(data.incomes.size() == 1 && data.expenses.size() == 0);
    CHECK(data.incomes[0].getId() == "INC0001");
    CHECK(data.incomes[0].getRecurringId() == "REC001");
    CHECK(data.incomes[0].getAmount() == 1000);
    // Day 31 does not exist in February 2026: the last day is used.
    CHECK(data.incomes[0].getDate() == Date(28, 2, 2026));
    CHECK(data.nextIncomeNo == 2);

    // Opening the app again in the same month adds nothing.
    CHECK(Recurring::generateDueTransactions(data, Date(27, 2, 2026)) == 0);
    CHECK(data.incomes.size() == 1);

    // Start month of REC002 (even on a day before its start day).
    CHECK(Recurring::generateDueTransactions(data, Date(1, 3, 2026)) == 2);
    CHECK(data.expenses.size() == 1);
    CHECK(data.expenses[0].getId() == "EXP0001");
    CHECK(data.expenses[0].getRecurringId() == "REC002");
    CHECK(data.expenses[0].getDate() == Date(5, 3, 2026));
    CHECK(data.incomes[1].getDate() == Date(31, 3, 2026));

    // End month of REC002 is still due, the month after is not.
    CHECK(Recurring::generateDueTransactions(data, Date(30, 6, 2026)) == 2);
    CHECK(Recurring::generateDueTransactions(data, Date(1, 7, 2026)) == 1);
    CHECK(data.expenses.size() == 2);
    CHECK(data.incomes.size() == 4);

    // A transaction that was entered in this month for REC001 counts as generated.
    data.incomes.add(Income("INC0099", Date(2, 8, 2026), "SRC001", 1000, "WAL001", "", "REC001"));
    CHECK(Recurring::generateDueTransactions(data, Date(20, 8, 2026)) == 0);

    // Same month in another year is a new month.
    CHECK(Recurring::generateDueTransactions(data, Date(1, 8, 2027)) == 1);

    // Invalid date: nothing happens.
    CHECK(Recurring::generateDueTransactions(data, Date(31, 2, 2026)) == 0);
}

static void testAnnualStatistics() {
    FinanceData data;
    data.sources.add(IncomeSource("SRC001", "Salary"));
    data.sources.add(IncomeSource("SRC002", "Bonus"));
    data.categories.add(ExpenseCategory("CAT001", "Food"));
    data.categories.add(ExpenseCategory("CAT002", "Rent"));
    data.wallets.add(Wallet("WAL001", "Cash"));

    DynamicArray<int> years;

    // No data: totals are 0 and there is one 0 per source / category.
    CHECK(AnnualStatistics::summarizeYears(data, years).totalIncome == 0);
    CHECK(AnnualStatistics::incomeBySource(data, years).size() == 2);
    CHECK(AnnualStatistics::expenseByCategory(data, years).size() == 2);

    data.incomes.add(Income("INC0001", Date(31, 12, 2025), "SRC001", 1000, "WAL001", ""));
    data.incomes.add(Income("INC0002", Date(1, 1, 2026), "SRC001", 2000, "WAL001", ""));
    data.incomes.add(Income("INC0003", Date(15, 6, 2026), "SRC002", 300, "WAL001", ""));
    data.incomes.add(Income("INC0004", Date(1, 1, 2024), "SRC002", 9000, "WAL001", ""));
    data.expenses.add(Expense("EXP0001", Date(10, 5, 2025), "CAT002", 700, "WAL001", ""));
    data.expenses.add(Expense("EXP0002", Date(10, 5, 2026), "CAT001", 50, "WAL001", ""));
    data.expenses.add(Expense("EXP0003", Date(11, 5, 2026), "CAT001", 25, "WAL001", ""));
    // Big amounts must not overflow (long long, not int).
    data.expenses.add(Expense("EXP0004", Date(1, 2, 2026), "CAT002", 3000000000LL, "WAL001", ""));

    // No year selected: nothing is counted.
    CHECK(AnnualStatistics::summarizeYears(data, years).totalExpense == 0);

    years.add(2026);
    Statistics::PeriodSummary one = AnnualStatistics::summarizeYears(data, years);
    CHECK(one.totalIncome == 2300);
    CHECK(one.totalExpense == 3000000075LL);
    CHECK(one.net() == 2300 - 3000000075LL);

    // Several years (not next to each other).
    years.add(2024);
    years.add(2025);
    Statistics::PeriodSummary many = AnnualStatistics::summarizeYears(data, years);
    CHECK(many.totalIncome == 12300);
    CHECK(many.totalExpense == 3000000775LL);

    DynamicArray<int> only2026;
    only2026.add(2026);
    DynamicArray<long long> bySource = AnnualStatistics::incomeBySource(data, only2026);
    CHECK(bySource.size() == 2);
    CHECK(bySource[0] == 2000);
    CHECK(bySource[1] == 300);
    DynamicArray<long long> byCategory = AnnualStatistics::expenseByCategory(data, only2026);
    CHECK(byCategory.size() == 2);
    CHECK(byCategory[0] == 75);
    CHECK(byCategory[1] == 3000000000LL);

    bySource = AnnualStatistics::incomeBySource(data, years);
    CHECK(bySource[0] == 3000);
    CHECK(bySource[1] == 9300);

    // A transaction whose source no longer exists is not put in any slot.
    data.incomes.add(Income("INC0005", Date(1, 3, 2026), "SRC999", 400, "WAL001", ""));
    bySource = AnnualStatistics::incomeBySource(data, only2026);
    CHECK(bySource[0] == 2000 && bySource[1] == 300);
}

static void testRecurringEdgeCases() {
    const std::string path = "bin/test_recurring.dat";
    FinanceData data;
    data.sources.add(IncomeSource("SRC001", "Salary"));
    data.categories.add(ExpenseCategory("CAT001", "Rent"));
    data.wallets.add(Wallet("WAL001", "Cash"));
    data.wallets.add(Wallet("WAL002", "Bank"));
    data.recurrings.add(RecurringTransaction("REC001", TransactionType::INCOME, "SRC001", 1000,
                                             "WAL002", "Pay", Date(31, 1, 2026), false, Date()));
    data.recurrings.add(RecurringTransaction("REC002", TransactionType::EXPENSE, "CAT001", 300,
                                             "WAL002", "Rent", Date(1, 1, 2026), false, Date()));

    // Open the app in March, close it (save), open it again (load) in the same month.
    CHECK(Recurring::generateDueTransactions(data, Date(3, 3, 2026)) == 2);
    CHECK(Storage::save(data, path));
    FinanceData reopened;
    CHECK(Storage::load(reopened, path));
    CHECK(Recurring::generateDueTransactions(reopened, Date(25, 3, 2026)) == 0);
    CHECK(reopened.incomes.size() == 1 && reopened.expenses.size() == 1);
    CHECK(reopened.incomes[0].getRecurringId() == "REC001");

    // After reopening, the IDs continue from the saved counters.
    CHECK(Recurring::generateDueTransactions(reopened, Date(1, 4, 2026)) == 2);
    CHECK(reopened.incomes[1].getId() == "INC0002");
    CHECK(reopened.expenses[1].getId() == "EXP0002");
    std::remove(path.c_str());

    // Generated transactions change the wallet balance.
    CHECK(Balance::walletBalance(reopened, "WAL002") == 2 * 1000 - 2 * 300);
    CHECK(Balance::walletBalance(reopened, "WAL001") == 0);

    // Day 31 in February of a leap year becomes the 29th.
    FinanceData leap = data;
    leap.incomes.clear();
    leap.expenses.clear();
    CHECK(Recurring::generateDueTransactions(leap, Date(1, 2, 2028)) == 2);
    CHECK(leap.incomes[0].getDate() == Date(29, 2, 2028));

    // Same month as the start date but one year earlier: not started yet.
    FinanceData later;
    later.sources.add(IncomeSource("SRC001", "Salary"));
    later.wallets.add(Wallet("WAL001", "Cash"));
    later.recurrings.add(RecurringTransaction("REC001", TransactionType::INCOME, "SRC001", 1000,
                                              "WAL001", "", Date(15, 3, 2027), false, Date()));
    CHECK(Recurring::generateDueTransactions(later, Date(20, 3, 2026)) == 0);
    CHECK(Recurring::generateDueTransactions(later, Date(20, 2, 2027)) == 0);
    CHECK(Recurring::generateDueTransactions(later, Date(1, 3, 2027)) == 1);

    // A deleted recurring item is no longer generated, but its old transactions stay.
    CHECK(Recurring::removeRecurring(reopened, 0));
    CHECK(Recurring::generateDueTransactions(reopened, Date(1, 5, 2026)) == 1);
    CHECK(reopened.incomes.size() == 2);
    CHECK(reopened.expenses.size() == 3);

    // An item whose wallet was removed is skipped instead of crashing.
    reopened.wallets.removeAt(1);
    CHECK(Recurring::generateDueTransactions(reopened, Date(1, 6, 2026)) == 0);

    // No recurring items at all.
    FinanceData empty;
    CHECK(Recurring::generateDueTransactions(empty, Date(1, 6, 2026)) == 0);
}

static void testAnnualStatisticsEdgeCases() {
    FinanceData data;
    data.sources.add(IncomeSource("SRC001", "Salary"));
    data.categories.add(ExpenseCategory("CAT001", "Rent"));
    data.wallets.add(Wallet("WAL001", "Cash"));
    data.incomes.add(Income("INC0001", Date(31, 12, 2026), "SRC001", 100, "WAL001", ""));
    data.incomes.add(Income("INC0002", Date(1, 1, 2027), "SRC001", 200, "WAL001", ""));
    data.expenses.add(Expense("EXP0001", Date(31, 12, 2026), "CAT001", 40, "WAL001", ""));

    // 31/12 and 01/01 belong to different years.
    DynamicArray<int> years;
    years.add(2026);
    CHECK(AnnualStatistics::summarizeYears(data, years).totalIncome == 100);
    CHECK(AnnualStatistics::summarizeYears(data, years).totalExpense == 40);

    // A year picked twice is still counted once.
    years.add(2026);
    CHECK(AnnualStatistics::summarizeYears(data, years).totalIncome == 100);
    CHECK(AnnualStatistics::incomeBySource(data, years)[0] == 100);

    // A selected year without transactions adds nothing.
    years.add(2030);
    CHECK(AnnualStatistics::summarizeYears(data, years).totalIncome == 100);

    // Only years without transactions: everything is 0.
    DynamicArray<int> none;
    none.add(1999);
    CHECK(AnnualStatistics::summarizeYears(data, none).net() == 0);
    CHECK(AnnualStatistics::incomeBySource(data, none)[0] == 0);
    CHECK(AnnualStatistics::expenseByCategory(data, none)[0] == 0);

    // No sources / categories: empty result.
    FinanceData empty;
    CHECK(AnnualStatistics::incomeBySource(empty, years).size() == 0);
    CHECK(AnnualStatistics::expenseByCategory(empty, years).size() == 0);

    // Totals follow edits and deletes.
    years.add(2027);
    CHECK(AnnualStatistics::summarizeYears(data, years).totalIncome == 300);
    Transactions::TransactionRef first;
    first.isIncome = true;
    first.index = 0;
    CHECK(Transactions::deleteTransaction(data, first));
    CHECK(AnnualStatistics::summarizeYears(data, years).totalIncome == 200);
}

static void testMasterDataDuplicateNames() {
    using MasterData::Kind;
    FinanceData data;
    std::string message;
    MasterData::addItem(data, Kind::WALLET, "Cash", message);          // WAL001
    MasterData::addItem(data, Kind::WALLET, "Bank Account", message);  // WAL002
    MasterData::addItem(data, Kind::CATEGORY, "Food", message);        // CAT001
    MasterData::addItem(data, Kind::SOURCE, "Salary", message);        // SRC001

    // addItem: same name, other case, extra spaces -> rejected, nothing changes.
    CHECK(!MasterData::addItem(data, Kind::WALLET, "Cash", message));
    CHECK(message != "");
    message = "";
    CHECK(!MasterData::addItem(data, Kind::WALLET, "CASH", message));
    CHECK(message != "");
    CHECK(!MasterData::addItem(data, Kind::WALLET, "  cash  ", message));
    CHECK(!MasterData::addItem(data, Kind::WALLET, "\tBank Account ", message));
    CHECK(data.wallets.size() == 2);
    CHECK(data.nextWalletNo == 3);

    // addItem: the same name in a different kind of list is allowed.
    CHECK(MasterData::addItem(data, Kind::CATEGORY, "Cash", message));
    CHECK(MasterData::addItem(data, Kind::SOURCE, "Food", message));
    CHECK(message == "");

    // renameItem: new name used by another item -> rejected, old name is kept.
    message = "";
    CHECK(!MasterData::renameItem(data, Kind::WALLET, "WAL002", "Cash", message));
    CHECK(message != "");
    CHECK(!MasterData::renameItem(data, Kind::WALLET, "WAL002", "cASh", message));
    CHECK(!MasterData::renameItem(data, Kind::WALLET, "WAL002", "  Cash ", message));
    CHECK(data.wallets[1].getName() == "Bank Account");

    // renameItem: empty or only spaces -> rejected.
    message = "";
    CHECK(!MasterData::renameItem(data, Kind::WALLET, "WAL002", "", message));
    CHECK(message != "");
    CHECK(!MasterData::renameItem(data, Kind::WALLET, "WAL002", "   ", message));
    CHECK(data.wallets[1].getName() == "Bank Account");

    // renameItem: a name used in another kind of list is fine.
    message = "x";
    CHECK(MasterData::renameItem(data, Kind::WALLET, "WAL002", "Food", message));
    CHECK(message == "");
    CHECK(data.wallets[1].getName() == "Food");

    // renameItem: the same name, or only another case, of the item itself is fine.
    CHECK(MasterData::renameItem(data, Kind::WALLET, "WAL001", "Cash", message));
    CHECK(MasterData::renameItem(data, Kind::WALLET, "WAL001", "CASH", message));
    CHECK(data.wallets[0].getName() == "CASH");

    // renameItem: spaces around the new name are removed.
    CHECK(MasterData::renameItem(data, Kind::SOURCE, "SRC001", "  Pay  ", message));
    CHECK(data.sources[0].getName() == "Pay");

    // renameItem: category can be renamed successfully.
    CHECK(MasterData::renameItem(data, Kind::CATEGORY, "CAT001", "  Groceries  ", message));
    CHECK(message == "");
    CHECK(data.categories[0].getName() == "Groceries");
    CHECK(data.categories[0].getId() == "CAT001");

    // renameItem: unknown id -> rejected with a message.
    message = "";
    CHECK(!MasterData::renameItem(data, Kind::WALLET, "WAL999", "Anything", message));
    CHECK(message != "");
    CHECK(!MasterData::renameItem(data, Kind::WALLET, "CAT001", "Anything", message));

    // The ID never changes when renaming.
    CHECK(data.wallets[0].getId() == "WAL001");
}

static void testMasterDataTabTrimming() {
    using MasterData::Kind;

    FinanceData data;
    std::string message;

    // A name containing only tabs must be rejected.
    CHECK(!MasterData::addItem(data, Kind::WALLET, "\t\t", message));
    CHECK(message != "");
    CHECK(data.wallets.isEmpty());

    // Tabs around a valid name must be trimmed.
    message = "";
    CHECK(MasterData::addItem(data, Kind::WALLET, "\tCash\t", message));
    CHECK(message == "");
    CHECK(data.wallets.size() == 1);
    CHECK(data.wallets[0].getName() == "Cash");

    // A name containing only tabs must also be rejected when renaming.
    message = "";
    CHECK(!MasterData::renameItem(data, Kind::WALLET, "WAL001", "\t\t", message));
    CHECK(message != "");
    CHECK(data.wallets[0].getName() == "Cash");

    // Tabs and spaces around a new name must be trimmed.
    message = "";
    CHECK(MasterData::renameItem(data, Kind::WALLET, "WAL001", "\t Bank Account \t", message));
    CHECK(message == "");
    CHECK(data.wallets[0].getName() == "Bank Account");
}

static void testMasterDataRemoveInUse() {
    using MasterData::Kind;
    FinanceData data;
    std::string message;
    MasterData::addItem(data, Kind::WALLET, "Cash", message);       // WAL001
    MasterData::addItem(data, Kind::WALLET, "Bank", message);       // WAL002
    MasterData::addItem(data, Kind::WALLET, "Unused", message);     // WAL003
    MasterData::addItem(data, Kind::CATEGORY, "Food", message);     // CAT001 (expense)
    MasterData::addItem(data, Kind::CATEGORY, "Rent", message);     // CAT002 (recurring)
    MasterData::addItem(data, Kind::CATEGORY, "Travel", message);   // CAT003 (unused)
    MasterData::addItem(data, Kind::SOURCE, "Salary", message);     // SRC001 (income)
    MasterData::addItem(data, Kind::SOURCE, "Bonus", message);      // SRC002 (recurring)
    MasterData::addItem(data, Kind::SOURCE, "Gift", message);       // SRC003 (unused)

    // Nothing is used yet.
    CHECK(!MasterData::isWalletInUse(data, "WAL001"));
    CHECK(!MasterData::isCategoryInUse(data, "CAT001"));
    CHECK(!MasterData::isSourceInUse(data, "SRC001"));

    data.incomes.add(Income("INC0001", Date(1, 1, 2026), "SRC001", 1000, "WAL001", ""));
    data.expenses.add(Expense("EXP0001", Date(2, 1, 2026), "CAT001", 100, "WAL001", ""));
    data.recurrings.add(RecurringTransaction("REC001", TransactionType::EXPENSE, "CAT002", 500,
                                             "WAL002", "", Date(1, 1, 2026), false, Date()));
    data.recurrings.add(RecurringTransaction("REC002", TransactionType::INCOME, "SRC002", 700,
                                             "WAL002", "", Date(1, 1, 2026), false, Date()));

    // The in-use checks see income, expense and recurring (both types).
    CHECK(MasterData::isSourceInUse(data, "SRC001"));    // income
    CHECK(MasterData::isSourceInUse(data, "SRC002"));    // recurring income
    CHECK(!MasterData::isSourceInUse(data, "SRC003"));
    CHECK(MasterData::isCategoryInUse(data, "CAT001"));  // expense
    CHECK(MasterData::isCategoryInUse(data, "CAT002"));  // recurring expense
    CHECK(!MasterData::isCategoryInUse(data, "CAT003"));
    CHECK(MasterData::isWalletInUse(data, "WAL001"));    // income + expense
    CHECK(MasterData::isWalletInUse(data, "WAL002"));    // recurring only
    CHECK(!MasterData::isWalletInUse(data, "WAL003"));

    // A recurring refId is only a category for EXPENSE and only a source for INCOME,
    // so the same text in the other list does not count as "in use".
    FinanceData other;
    other.recurrings.add(RecurringTransaction("REC001", TransactionType::INCOME, "X001", 1,
                                              "W", "", Date(1, 1, 2026), false, Date()));
    CHECK(MasterData::isSourceInUse(other, "X001"));
    CHECK(!MasterData::isCategoryInUse(other, "X001"));

    // Removing an item in use is refused: message is set and nothing changes.
    message = "";
    CHECK(!MasterData::removeItem(data, Kind::SOURCE, "SRC001", message));      // income
    CHECK(message != "");
    message = "";
    CHECK(!MasterData::removeItem(data, Kind::CATEGORY, "CAT001", message));    // expense
    CHECK(message != "");
    message = "";
    CHECK(!MasterData::removeItem(data, Kind::CATEGORY, "CAT002", message));    // recurring
    CHECK(message != "");
    message = "";
    CHECK(!MasterData::removeItem(data, Kind::SOURCE, "SRC002", message));      // recurring
    CHECK(message != "");
    message = "";
    CHECK(!MasterData::removeItem(data, Kind::WALLET, "WAL001", message));      // transactions
    CHECK(message != "");
    message = "";
    CHECK(!MasterData::removeItem(data, Kind::WALLET, "WAL002", message));      // recurring only
    CHECK(message != "");
    CHECK(data.wallets.size() == 3 && data.categories.size() == 3 && data.sources.size() == 3);

    // Removing an unknown id is refused with a message.
    message = "";
    CHECK(!MasterData::removeItem(data, Kind::WALLET, "WAL999", message));
    CHECK(message != "");
    CHECK(!MasterData::removeItem(data, Kind::WALLET, "CAT003", message));
    CHECK(data.wallets.size() == 3 && data.categories.size() == 3);

    // Removing an unused item works, with an empty message.
    message = "x";
    CHECK(MasterData::removeItem(data, Kind::WALLET, "WAL003", message));
    CHECK(message == "");
    CHECK(data.wallets.size() == 2);
    CHECK(MasterData::findWalletIndex(data, "WAL003") == -1);
    CHECK(MasterData::removeItem(data, Kind::CATEGORY, "CAT003", message));
    CHECK(MasterData::removeItem(data, Kind::SOURCE, "SRC003", message));
    CHECK(data.categories.size() == 2 && data.sources.size() == 2);

    // Removing the first item keeps the others (and their order).
    CHECK(MasterData::removeItem(data, Kind::CATEGORY, "CAT001", message) == false);
    data.expenses.removeAt(0);
    CHECK(MasterData::removeItem(data, Kind::CATEGORY, "CAT001", message));
    CHECK(data.categories.size() == 1);
    CHECK(data.categories[0].getId() == "CAT002");

    // After the transactions are gone the item can be removed too.
    data.recurrings.removeAt(0);
    CHECK(MasterData::removeItem(data, Kind::CATEGORY, "CAT002", message));
    CHECK(data.categories.isEmpty());

    // A deleted ID is never reused: the counter is not decreased.
    CHECK(data.nextWalletNo == 4);
    CHECK(MasterData::addItem(data, Kind::WALLET, "New", message));
    CHECK(data.wallets[data.wallets.size() - 1].getId() == "WAL004");
    CHECK(MasterData::addItem(data, Kind::CATEGORY, "Again", message));
    CHECK(data.categories[0].getId() == "CAT004");
}

static void testMasterDataSourceIdNotReused() {
    using MasterData::Kind;

    FinanceData data;
    std::string message;

    // Add three income sources.
    CHECK(MasterData::addItem(data, Kind::SOURCE, "Salary", message));
    CHECK(MasterData::addItem(data, Kind::SOURCE, "Bonus", message));
    CHECK(MasterData::addItem(data, Kind::SOURCE, "Gift", message));

    CHECK(data.sources.size() == 3);
    CHECK(data.sources[1].getId() == "SRC002");

    // Remove the middle source.
    CHECK(MasterData::removeItem(data, Kind::SOURCE, "SRC002", message));
    CHECK(message == "");
    CHECK(data.sources.size() == 2);

    // Add a new source. The deleted ID must not be reused.
    CHECK(MasterData::addItem(data, Kind::SOURCE, "Freelance", message));
    CHECK(message == "");
    CHECK(data.sources.size() == 3);

    CHECK(data.sources[2].getId() == "SRC004");
    CHECK(MasterData::findSourceIndex(data, "SRC002") == -1);
    CHECK(MasterData::findSourceIndex(data, "SRC004") == 2);
}

static void testMasterDataRemoveMiddleItem() {
    using MasterData::Kind;

    FinanceData data;
    std::string message;

    // Create three items in each list.
    MasterData::addItem(data, Kind::CATEGORY, "Food", message);
    MasterData::addItem(data, Kind::CATEGORY, "Rent", message);
    MasterData::addItem(data, Kind::CATEGORY, "Travel", message);

    MasterData::addItem(data, Kind::SOURCE, "Salary", message);
    MasterData::addItem(data, Kind::SOURCE, "Bonus", message);
    MasterData::addItem(data, Kind::SOURCE, "Gift", message);

    MasterData::addItem(data, Kind::WALLET, "Cash", message);
    MasterData::addItem(data, Kind::WALLET, "Bank", message);
    MasterData::addItem(data, Kind::WALLET, "Savings", message);

    // Remove the middle item from each list.
    CHECK(MasterData::removeItem(data, Kind::CATEGORY, "CAT002", message));
    CHECK(MasterData::removeItem(data, Kind::SOURCE, "SRC002", message));
    CHECK(MasterData::removeItem(data, Kind::WALLET, "WAL002", message));

    // Check the remaining items and their order.
    CHECK(data.categories.size() == 2);
    CHECK(data.categories[0].getId() == "CAT001");
    CHECK(data.categories[1].getId() == "CAT003");

    CHECK(data.sources.size() == 2);
    CHECK(data.sources[0].getId() == "SRC001");
    CHECK(data.sources[1].getId() == "SRC003");

    CHECK(data.wallets.size() == 2);
    CHECK(data.wallets[0].getId() == "WAL001");
    CHECK(data.wallets[1].getId() == "WAL003");

    // The deleted IDs can no longer be found.
    CHECK(MasterData::findCategoryIndex(data, "CAT002") == -1);
    CHECK(MasterData::findSourceIndex(data, "SRC002") == -1);
    CHECK(MasterData::findWalletIndex(data, "WAL002") == -1);

    // The remaining IDs are found at their new positions.
    CHECK(MasterData::findCategoryIndex(data, "CAT003") == 1);
    CHECK(MasterData::findSourceIndex(data, "SRC003") == 1);
    CHECK(MasterData::findWalletIndex(data, "WAL003") == 1);
}

static void testMasterDataInUseAfterTransactionDeletion() {
    using MasterData::Kind;

    FinanceData data;
    std::string message;

    // Create master data.
    CHECK(MasterData::addItem(data, Kind::SOURCE, "Salary", message));
    CHECK(MasterData::addItem(data, Kind::CATEGORY, "Food", message));
    CHECK(MasterData::addItem(data, Kind::WALLET, "Cash", message));
    CHECK(MasterData::addItem(data, Kind::WALLET, "Bank", message));

    // Create one income and one expense in different wallets.
    data.incomes.add(Income(
        "INC0001", Date(1, 3, 2026), "SRC001", 1000, "WAL001", ""
    ));

    data.expenses.add(Expense(
        "EXP0001", Date(2, 3, 2026), "CAT001", 200, "WAL002", ""
    ));

    // Both transactions make their related items in use.
    CHECK(MasterData::isSourceInUse(data, "SRC001"));
    CHECK(MasterData::isCategoryInUse(data, "CAT001"));
    CHECK(MasterData::isWalletInUse(data, "WAL001"));
    CHECK(MasterData::isWalletInUse(data, "WAL002"));

    // Delete the income.
    Transactions::TransactionRef income = {true, 0};
    CHECK(Transactions::deleteTransaction(data, income));

    // Its source and wallet are no longer in use.
    CHECK(!MasterData::isSourceInUse(data, "SRC001"));
    CHECK(!MasterData::isWalletInUse(data, "WAL001"));

    // The expense still uses its category and wallet.
    CHECK(MasterData::isCategoryInUse(data, "CAT001"));
    CHECK(MasterData::isWalletInUse(data, "WAL002"));

    // Delete the expense.
    Transactions::TransactionRef expense = {false, 0};
    CHECK(Transactions::deleteTransaction(data, expense));

    // All related items are now unused.
    CHECK(!MasterData::isCategoryInUse(data, "CAT001"));
    CHECK(!MasterData::isWalletInUse(data, "WAL002"));
}

static void testMasterDataInUseAfterIncomeUpdate() {
    using MasterData::Kind;

    FinanceData data;
    std::string message;

    // Create two sources and two wallets.
    CHECK(MasterData::addItem(data, Kind::SOURCE, "Salary", message));
    CHECK(MasterData::addItem(data, Kind::SOURCE, "Bonus", message));
    CHECK(MasterData::addItem(data, Kind::WALLET, "Cash", message));
    CHECK(MasterData::addItem(data, Kind::WALLET, "Bank", message));

    // Create an income using the first source and wallet.
    data.incomes.add(Income(
        "INC001", Date(1, 1, 2025), "SRC001",
        100000, "WAL001", "Original income"
    ));

    // Initially, the first source and wallet are in use.
    CHECK(MasterData::isSourceInUse(data, "SRC001"));
    CHECK(MasterData::isWalletInUse(data, "WAL001"));

    CHECK(!MasterData::isSourceInUse(data, "SRC002"));
    CHECK(!MasterData::isWalletInUse(data, "WAL002"));

    // Update the income to use the second source and wallet.
    Income updatedIncome(
        "INC001", Date(2, 1, 2025), "SRC002",
        200000, "WAL002", "Updated income"
    );

    CHECK(Transactions::updateIncome(data, 0, updatedIncome));

    // The old source and wallet are no longer in use.
    CHECK(!MasterData::isSourceInUse(data, "SRC001"));
    CHECK(!MasterData::isWalletInUse(data, "WAL001"));

    // The new source and wallet are now in use.
    CHECK(MasterData::isSourceInUse(data, "SRC002"));
    CHECK(MasterData::isWalletInUse(data, "WAL002"));
}

static void testMasterDataInUseAfterExpenseUpdate() {
    using MasterData::Kind;

    FinanceData data;
    std::string message;

    // Create two categories and two wallets.
    CHECK(MasterData::addItem(data, Kind::CATEGORY, "Food", message));
    CHECK(MasterData::addItem(data, Kind::CATEGORY, "Transport", message));
    CHECK(MasterData::addItem(data, Kind::WALLET, "Cash", message));
    CHECK(MasterData::addItem(data, Kind::WALLET, "Bank", message));

    // Create an expense using the first category and wallet.
    data.expenses.add(Expense(
        "EXP001", Date(1, 1, 2025), "CAT001",
        50000, "WAL001", "Original expense"
    ));

    // Initially, the first category and wallet are in use.
    CHECK(MasterData::isCategoryInUse(data, "CAT001"));
    CHECK(MasterData::isWalletInUse(data, "WAL001"));

    CHECK(!MasterData::isCategoryInUse(data, "CAT002"));
    CHECK(!MasterData::isWalletInUse(data, "WAL002"));

    // Update the expense to use the second category and wallet.
    Expense updatedExpense(
        "EXP001", Date(2, 1, 2025), "CAT002",
        80000, "WAL002", "Updated expense"
    );

    CHECK(Transactions::updateExpense(data, 0, updatedExpense));

    // The old category and wallet are no longer in use.
    CHECK(!MasterData::isCategoryInUse(data, "CAT001"));
    CHECK(!MasterData::isWalletInUse(data, "WAL001"));

    // The new category and wallet are now in use.
    CHECK(MasterData::isCategoryInUse(data, "CAT002"));
    CHECK(MasterData::isWalletInUse(data, "WAL002"));
}

static void testBalanceAfterTransactionChanges() {
    FinanceData data;
    data.sources.add(IncomeSource("SRC001", "Salary"));
    data.categories.add(ExpenseCategory("CAT001", "Food"));
    data.wallets.add(Wallet("WAL001", "Cash"));
    data.wallets.add(Wallet("WAL002", "Bank"));

    // Cash: +1000 +2000 +3000 -400 -500 -600.  Bank: nothing.
    data.incomes.add(Income("INC0001", Date(1, 3, 2026), "SRC001", 1000, "WAL001", ""));
    data.incomes.add(Income("INC0002", Date(2, 3, 2026), "SRC001", 2000, "WAL001", ""));
    data.incomes.add(Income("INC0003", Date(3, 3, 2026), "SRC001", 3000, "WAL001", ""));
    data.expenses.add(Expense("EXP0001", Date(4, 3, 2026), "CAT001", 400, "WAL001", ""));
    data.expenses.add(Expense("EXP0002", Date(5, 3, 2026), "CAT001", 500, "WAL001", ""));
    data.expenses.add(Expense("EXP0003", Date(6, 3, 2026), "CAT001", 600, "WAL001", ""));
    CHECK(Balance::walletBalance(data, "WAL001") == 4500);
    CHECK(Balance::totalBalance(data) == 4500);

    // Change the amount of an income: the balance changes by the difference.
    CHECK(Transactions::updateIncome(data, 1, Income("INC0002", Date(2, 3, 2026), "SRC001",
                                                     2500, "WAL001", "")));
    CHECK(Balance::walletBalance(data, "WAL001") == 5000);

    // Change the amount of an expense (a bigger expense lowers the balance).
    CHECK(Transactions::updateExpense(data, 0, Expense("EXP0001", Date(4, 3, 2026), "CAT001",
                                                       900, "WAL001", "")));
    CHECK(Balance::walletBalance(data, "WAL001") == 4500);

    // Move an income to another wallet: one wallet loses it, the other gets it.
    CHECK(Transactions::updateIncome(data, 2, Income("INC0003", Date(3, 3, 2026), "SRC001",
                                                     3000, "WAL002", "")));
    CHECK(Balance::walletBalance(data, "WAL001") == 1500);
    CHECK(Balance::walletBalance(data, "WAL002") == 3000);
    CHECK(Balance::totalBalance(data) == 4500);   // the total does not change

    // Move an expense to another wallet.
    CHECK(Transactions::updateExpense(data, 1, Expense("EXP0002", Date(5, 3, 2026), "CAT001",
                                                       500, "WAL002", "")));
    CHECK(Balance::walletBalance(data, "WAL001") == 2000);
    CHECK(Balance::walletBalance(data, "WAL002") == 2500);
    CHECK(Balance::totalBalance(data) == 4500);

    // A rejected update (unknown wallet) must not change any balance.
    CHECK(!Transactions::updateIncome(data, 0, Income("INC0001", Date(1, 3, 2026), "SRC001",
                                                      99999, "WAL999", "")));
    CHECK(Balance::walletBalance(data, "WAL001") == 2000);

    // Delete at the start, in the middle and at the end of the lists (incomes first).
    // Now: incomes INC0001(1000,W1) INC0002(2500,W1) INC0003(3000,W2)
    //      expenses EXP0001(900,W1) EXP0002(500,W2) EXP0003(600,W1)
    Transactions::TransactionRef first = {true, 0};
    CHECK(Transactions::deleteTransaction(data, first));      // INC0001 (+1000, W1)
    CHECK(Balance::walletBalance(data, "WAL001") == 1000);
    Transactions::TransactionRef middle = {true, 1};
    CHECK(Transactions::deleteTransaction(data, middle));     // INC0003 (+3000, W2)
    CHECK(Balance::walletBalance(data, "WAL002") == -500);
    Transactions::TransactionRef last = {false, 2};
    CHECK(Transactions::deleteTransaction(data, last));       // EXP0003 (-600, W1)
    CHECK(Balance::walletBalance(data, "WAL001") == 1600);
    Transactions::TransactionRef expSecond = {false, 1};
    CHECK(Transactions::deleteTransaction(data, expSecond));  // EXP0002 (-500, W2)
    CHECK(Balance::walletBalance(data, "WAL002") == 0);
    Transactions::TransactionRef expFirst = {false, 0};
    CHECK(Transactions::deleteTransaction(data, expFirst));   // EXP0001 (-900, W1)
    CHECK(Balance::walletBalance(data, "WAL001") == 2500);    // only INC0002 is left
    CHECK(Balance::totalBalance(data) == 2500);

    // Delete the last transaction: both wallets are back to 0.
    Transactions::TransactionRef onlyIncome = {true, 0};
    CHECK(Transactions::deleteTransaction(data, onlyIncome));
    CHECK(Balance::walletBalance(data, "WAL001") == 0);
    CHECK(Balance::totalBalance(data) == 0);
}

static void testBalanceNegativeWallet() {
    FinanceData data;
    data.wallets.add(Wallet("WAL001", "Cash"));
    data.wallets.add(Wallet("WAL002", "Credit Card"));
    data.wallets.add(Wallet("WAL003", "Bank"));

    // Spend more than the wallet has: the balance is negative, not 0.
    data.incomes.add(Income("INC0001", Date(1, 4, 2026), "SRC001", 1000, "WAL001", ""));
    data.expenses.add(Expense("EXP0001", Date(2, 4, 2026), "CAT001", 1500, "WAL001", ""));
    CHECK(Balance::walletBalance(data, "WAL001") == -500);
    CHECK(Balance::totalBalance(data) == -500);

    // An expense in a wallet with no income at all.
    data.expenses.add(Expense("EXP0002", Date(3, 4, 2026), "CAT001", 2000, "WAL002", ""));
    CHECK(Balance::walletBalance(data, "WAL002") == -2000);
    CHECK(Balance::totalBalance(data) == -2500);

    // Total with a negative wallet next to a positive one: they cancel each other.
    data.incomes.add(Income("INC0002", Date(4, 4, 2026), "SRC001", 10000, "WAL003", ""));
    CHECK(Balance::walletBalance(data, "WAL003") == 10000);
    CHECK(Balance::totalBalance(data) == 7500);

    // Exactly zero is not negative.
    data.expenses.add(Expense("EXP0003", Date(5, 4, 2026), "CAT001", 500, "WAL001", ""));
    data.incomes.add(Income("INC0003", Date(6, 4, 2026), "SRC001", 1000, "WAL001", ""));
    CHECK(Balance::walletBalance(data, "WAL001") == 0);

    // Income brings a negative wallet back up (-2000 + 2500 = 500).
    data.incomes.add(Income("INC0004", Date(7, 4, 2026), "SRC001", 2500, "WAL002", ""));
    CHECK(Balance::walletBalance(data, "WAL002") == 500);

    // Big negative amount: long long must not overflow.
    data.expenses.add(Expense("EXP0004", Date(8, 4, 2026), "CAT001", 6000000000LL, "WAL003", ""));
    CHECK(Balance::walletBalance(data, "WAL003") == 10000 - 6000000000LL);
    CHECK(Balance::totalBalance(data) == 0 + 500 + 10000 - 6000000000LL);

    // Every wallet negative: the total is negative too.
    FinanceData broke;
    broke.wallets.add(Wallet("WAL001", "A"));
    broke.wallets.add(Wallet("WAL002", "B"));
    broke.expenses.add(Expense("EXP0001", Date(1, 4, 2026), "CAT001", 100, "WAL001", ""));
    broke.expenses.add(Expense("EXP0002", Date(1, 4, 2026), "CAT001", 250, "WAL002", ""));
    CHECK(Balance::walletBalance(broke, "WAL001") == -100);
    CHECK(Balance::walletBalance(broke, "WAL002") == -250);
    CHECK(Balance::totalBalance(broke) == -350);
}

static void testStorageEdgeCases() {
    std::string path = "test_empty.dat";
    std::remove(path.c_str());

    FinanceData original;
    CHECK(Storage::save(original, path));

    FinanceData loaded;
    loaded.nextCategoryNo = 100;
    loaded.nextSourceNo = 100;
    loaded.nextWalletNo = 100;
    loaded.nextIncomeNo = 100;
    loaded.nextExpenseNo = 100;
    loaded.nextRecurringNo = 100;

    CHECK(Storage::load(loaded, path));

    CHECK(loaded.categories.isEmpty());
    CHECK(loaded.sources.isEmpty());
    CHECK(loaded.wallets.isEmpty());
    CHECK(loaded.incomes.isEmpty());
    CHECK(loaded.expenses.isEmpty());
    CHECK(loaded.recurrings.isEmpty());

    CHECK(loaded.categories.size() == 0);
    CHECK(loaded.sources.size() == 0);
    CHECK(loaded.wallets.size() == 0);
    CHECK(loaded.incomes.size() == 0);
    CHECK(loaded.expenses.size() == 0);
    CHECK(loaded.recurrings.size() == 0);

    CHECK(loaded.nextCategoryNo == original.nextCategoryNo);
    CHECK(loaded.nextSourceNo == original.nextSourceNo);
    CHECK(loaded.nextWalletNo == original.nextWalletNo);
    CHECK(loaded.nextIncomeNo == original.nextIncomeNo);
    CHECK(loaded.nextExpenseNo == original.nextExpenseNo);
    CHECK(loaded.nextRecurringNo == original.nextRecurringNo);

    std::remove(path.c_str());

    std::string fullPath = "test_full.dat";
    std::remove(fullPath.c_str());

    FinanceData dataFull;
    dataFull.nextCategoryNo = 42;
    dataFull.nextSourceNo = 53;
    dataFull.nextWalletNo = 64;
    dataFull.nextIncomeNo = 1005;
    dataFull.nextExpenseNo = 2006;
    dataFull.nextRecurringNo = 77;

    dataFull.wallets.add(Wallet("WAL001", "Cash"));
    dataFull.wallets.add(Wallet("WAL002", ""));
    dataFull.wallets.add(Wallet("WAL003", "Main Bank Account"));

    dataFull.categories.add(ExpenseCategory("CAT001", "Food & Dining"));
    dataFull.categories.add(ExpenseCategory("CAT002", ""));

    dataFull.sources.add(IncomeSource("SRC001", "Salary"));
    dataFull.sources.add(IncomeSource("SRC002", ""));

    dataFull.incomes.add(Income("INC0001", Date(1, 1, 2026), "SRC001", 9999999999999LL, "WAL001", "", ""));
    dataFull.incomes.add(Income("INC0002", Date(15, 6, 2026), "SRC002", 5000000LL, "WAL003", "Bonus salary", "REC001"));

    dataFull.expenses.add(Expense("EXP0001", Date(2, 1, 2026), "CAT001", 8888888888888LL, "WAL001", "", ""));
    dataFull.expenses.add(Expense("EXP0002", Date(20, 8, 2026), "CAT002", 120000LL, "WAL002", "Office supplies", "REC002"));

    dataFull.recurrings.add(RecurringTransaction("REC001", TransactionType::INCOME, "SRC001", 1234567890123LL, "WAL003", "", Date(1, 1, 2026), false, Date(1, 1, 2000)));
    dataFull.recurrings.add(RecurringTransaction("REC002", TransactionType::EXPENSE, "CAT001", 3500000LL, "WAL001", "Monthly rent", Date(5, 2, 2026), true, Date(5, 12, 2026)));

    CHECK(Storage::save(dataFull, fullPath));

    FinanceData loadedFull;
    CHECK(Storage::load(loadedFull, fullPath));

    CHECK(loadedFull.nextCategoryNo == dataFull.nextCategoryNo);
    CHECK(loadedFull.nextSourceNo == dataFull.nextSourceNo);
    CHECK(loadedFull.nextWalletNo == dataFull.nextWalletNo);
    CHECK(loadedFull.nextIncomeNo == dataFull.nextIncomeNo);
    CHECK(loadedFull.nextExpenseNo == dataFull.nextExpenseNo);
    CHECK(loadedFull.nextRecurringNo == dataFull.nextRecurringNo);

    CHECK(loadedFull.wallets.size() == dataFull.wallets.size());
    for (int i = 0; i < dataFull.wallets.size(); i++) {
        CHECK(loadedFull.wallets[i].getId() == dataFull.wallets[i].getId());
        CHECK(loadedFull.wallets[i].getName() == dataFull.wallets[i].getName());
    }

    CHECK(loadedFull.categories.size() == dataFull.categories.size());
    for (int i = 0; i < dataFull.categories.size(); i++) {
        CHECK(loadedFull.categories[i].getId() == dataFull.categories[i].getId());
        CHECK(loadedFull.categories[i].getName() == dataFull.categories[i].getName());
    }

    CHECK(loadedFull.sources.size() == dataFull.sources.size());
    for (int i = 0; i < dataFull.sources.size(); i++) {
        CHECK(loadedFull.sources[i].getId() == dataFull.sources[i].getId());
        CHECK(loadedFull.sources[i].getName() == dataFull.sources[i].getName());
    }

    CHECK(loadedFull.incomes.size() == dataFull.incomes.size());
    for (int i = 0; i < dataFull.incomes.size(); i++) {
        CHECK(loadedFull.incomes[i].getId() == dataFull.incomes[i].getId());
        CHECK(loadedFull.incomes[i].getDate() == dataFull.incomes[i].getDate());
        CHECK(loadedFull.incomes[i].getSourceId() == dataFull.incomes[i].getSourceId());
        CHECK(loadedFull.incomes[i].getAmount() == dataFull.incomes[i].getAmount());
        CHECK(loadedFull.incomes[i].getWalletId() == dataFull.incomes[i].getWalletId());
        CHECK(loadedFull.incomes[i].getDescription() == dataFull.incomes[i].getDescription());
        CHECK(loadedFull.incomes[i].getRecurringId() == dataFull.incomes[i].getRecurringId());
    }

    CHECK(loadedFull.expenses.size() == dataFull.expenses.size());
    for (int i = 0; i < dataFull.expenses.size(); i++) {
        CHECK(loadedFull.expenses[i].getId() == dataFull.expenses[i].getId());
        CHECK(loadedFull.expenses[i].getDate() == dataFull.expenses[i].getDate());
        CHECK(loadedFull.expenses[i].getCategoryId() == dataFull.expenses[i].getCategoryId());
        CHECK(loadedFull.expenses[i].getAmount() == dataFull.expenses[i].getAmount());
        CHECK(loadedFull.expenses[i].getWalletId() == dataFull.expenses[i].getWalletId());
        CHECK(loadedFull.expenses[i].getDescription() == dataFull.expenses[i].getDescription());
        CHECK(loadedFull.expenses[i].getRecurringId() == dataFull.expenses[i].getRecurringId());
    }

    CHECK(loadedFull.recurrings.size() == dataFull.recurrings.size());
    for (int i = 0; i < dataFull.recurrings.size(); i++) {
        CHECK(loadedFull.recurrings[i].getId() == dataFull.recurrings[i].getId());
        CHECK(loadedFull.recurrings[i].getType() == dataFull.recurrings[i].getType());
        CHECK(loadedFull.recurrings[i].getRefId() == dataFull.recurrings[i].getRefId());
        CHECK(loadedFull.recurrings[i].getAmount() == dataFull.recurrings[i].getAmount());
        CHECK(loadedFull.recurrings[i].getWalletId() == dataFull.recurrings[i].getWalletId());
        CHECK(loadedFull.recurrings[i].getDescription() == dataFull.recurrings[i].getDescription());
        CHECK(loadedFull.recurrings[i].getStartDate() == dataFull.recurrings[i].getStartDate());
        CHECK(loadedFull.recurrings[i].getHasEndDate() == dataFull.recurrings[i].getHasEndDate());
        CHECK(loadedFull.recurrings[i].getEndDate() == dataFull.recurrings[i].getEndDate());
    }

    std::remove(fullPath.c_str());

    FinanceData nonExistentData;
    CHECK(!Storage::load(nonExistentData, "non_existent_file_path_12345.dat"));

    std::string emptyPath = "test_empty_zero_byte.dat";
    std::remove(emptyPath.c_str());
    {
        std::ofstream emptyFile(emptyPath, std::ios::binary);
    }
    FinanceData emptyZeroData;
    CHECK(!Storage::load(emptyZeroData, emptyPath));
    std::remove(emptyPath.c_str());

    std::string corruptGarbagePath = "test_corrupt_garbage.dat";
    std::remove(corruptGarbagePath.c_str());
    {
        std::ofstream f(corruptGarbagePath, std::ios::binary);
        const char garbage[] = "\xff\xff\xff\xff\x00\x00\x00\x00\xfe\xed\xfa\xce\x12\x34\x56\x78";
        f.write(garbage, sizeof(garbage));
    }
    FinanceData corruptGarbageData;
    CHECK(!Storage::load(corruptGarbageData, corruptGarbagePath));
    std::remove(corruptGarbagePath.c_str());

    std::string corruptHugeStrPath = "test_corrupt_huge_string.dat";
    std::remove(corruptHugeStrPath.c_str());
    {
        std::ofstream f(corruptHugeStrPath, std::ios::binary);
        int val = 1;
        for (int i = 0; i < 6; i++) {
            f.write(reinterpret_cast<const char*>(&val), sizeof(val));
        }
        int count = 1;
        f.write(reinterpret_cast<const char*>(&count), sizeof(count));
        int hugeLen = 1000000;
        f.write(reinterpret_cast<const char*>(&hugeLen), sizeof(hugeLen));
    }
    FinanceData corruptHugeStrData;
    CHECK(!Storage::load(corruptHugeStrData, corruptHugeStrPath));
    std::remove(corruptHugeStrPath.c_str());

    std::string corruptNegCountPath = "test_corrupt_neg_count.dat";
    std::remove(corruptNegCountPath.c_str());
    {
        std::ofstream f(corruptNegCountPath, std::ios::binary);
        int val = 1;
        for (int i = 0; i < 6; i++) {
            f.write(reinterpret_cast<const char*>(&val), sizeof(val));
        }
        int negCount = -5;
        f.write(reinterpret_cast<const char*>(&negCount), sizeof(negCount));
    }
    FinanceData corruptNegCountData;
    CHECK(!Storage::load(corruptNegCountData, corruptNegCountPath));
    std::remove(corruptNegCountPath.c_str());

    FinanceData validData;
    validData.categories.add(ExpenseCategory("CAT001", "Groceries"));
    validData.sources.add(IncomeSource("SRC001", "Salary"));
    validData.wallets.add(Wallet("WAL001", "Bank"));
    validData.incomes.add(Income("INC0001", Date(1, 1, 2026), "SRC001", 1000000LL, "WAL001", "Monthly income", ""));
    validData.expenses.add(Expense("EXP0001", Date(2, 1, 2026), "CAT001", 50000LL, "WAL001", "Food", ""));
    validData.recurrings.add(RecurringTransaction("REC001", TransactionType::EXPENSE, "CAT001", 50000LL, "WAL001", "Food", Date(1, 1, 2026), false, Date(1, 1, 2000)));

    std::string validPath = "test_valid_for_trunc.dat";
    std::remove(validPath.c_str());
    CHECK(Storage::save(validData, validPath));

    std::ifstream in(validPath, std::ios::binary | std::ios::ate);
    std::streamsize fileSize = in.tellg();
    in.seekg(0, std::ios::beg);
    char* fileBuffer = new char[fileSize];
    in.read(fileBuffer, fileSize);
    in.close();

    std::string truncFewBytesPath = "test_trunc_few.dat";
    std::remove(truncFewBytesPath.c_str());
    {
        std::ofstream out(truncFewBytesPath, std::ios::binary);
        out.write(fileBuffer, 10);
    }
    FinanceData truncFewData;
    CHECK(!Storage::load(truncFewData, truncFewBytesPath));
    std::remove(truncFewBytesPath.c_str());

    std::string truncHalfPath = "test_trunc_half.dat";
    std::remove(truncHalfPath.c_str());
    {
        std::ofstream out(truncHalfPath, std::ios::binary);
        out.write(fileBuffer, fileSize / 2);
    }
    FinanceData truncHalfData;
    CHECK(!Storage::load(truncHalfData, truncHalfPath));
    std::remove(truncHalfPath.c_str());

    std::string truncAlmostPath = "test_trunc_almost.dat";
    std::remove(truncAlmostPath.c_str());
    {
        std::ofstream out(truncAlmostPath, std::ios::binary);
        out.write(fileBuffer, fileSize - 1);
    }
    FinanceData truncAlmostData;
    CHECK(!Storage::load(truncAlmostData, truncAlmostPath));
    std::remove(truncAlmostPath.c_str());

    delete[] fileBuffer;
    std::remove(validPath.c_str());
}

// ===== Week 4: cross test of Transactions (Kien's module) =====
static void testCrossAddTransactions() {
    FinanceData data;
    data.sources.add(IncomeSource("SRC001", "Salary"));
    data.categories.add(ExpenseCategory("CAT001", "Food"));
    data.wallets.add(Wallet("WAL001", "Cash"));
    Date day(10, 11, 2026);

    // The first IDs have 4 digits and start at 0001.
    CHECK(Transactions::addIncomeRecord(data, day, "SRC001", 5000000, "WAL001", "Salary") == "INC0001");
    CHECK(Transactions::addExpenseRecord(data, day, "CAT001", 30000, "WAL001", "Lunch") == "EXP0001");
    CHECK(Transactions::addIncomeRecord(data, day, "SRC001", 1, "WAL001", "") == "INC0002");
    CHECK(data.incomes.size() == 2);
    CHECK(data.expenses.size() == 1);
    CHECK(data.nextIncomeNo == 3);
    CHECK(data.nextExpenseNo == 2);

    // The saved transaction keeps every value that was passed in.
    CHECK(data.incomes[0].getDate() == day);
    CHECK(data.incomes[0].getSourceId() == "SRC001");
    CHECK(data.incomes[0].getAmount() == 5000000);
    CHECK(data.incomes[0].getWalletId() == "WAL001");
    CHECK(data.incomes[0].getDescription() == "Salary");
    CHECK(data.incomes[0].getRecurringId() == "");
    CHECK(data.expenses[0].getCategoryId() == "CAT001");
    CHECK(data.expenses[0].getDescription() == "Lunch");

    // An empty description is allowed.
    CHECK(data.incomes[1].getDescription() == "");

    // Amounts above the int range must not overflow.
    CHECK(Transactions::addExpenseRecord(data, day, "CAT001", 5000000000LL, "WAL001", "Big") == "EXP0002");
    CHECK(data.expenses[1].getAmount() == 5000000000LL);

    // Bad input: nothing is added and no ID is used up.
    int incomesBefore = data.incomes.size();
    int expensesBefore = data.expenses.size();
    int nextIncomeBefore = data.nextIncomeNo;
    int nextExpenseBefore = data.nextExpenseNo;

    CHECK(Transactions::addIncomeRecord(data, day, "SRC001", 0, "WAL001", "") == "");
    CHECK(Transactions::addIncomeRecord(data, day, "SRC001", -100, "WAL001", "") == "");
    CHECK(Transactions::addIncomeRecord(data, Date(31, 4, 2026), "SRC001", 100, "WAL001", "") == "");
    CHECK(Transactions::addIncomeRecord(data, Date(29, 2, 2026), "SRC001", 100, "WAL001", "") == "");
    CHECK(Transactions::addIncomeRecord(data, Date(1, 1, 1800), "SRC001", 100, "WAL001", "") == "");
    CHECK(Transactions::addIncomeRecord(data, day, "", 100, "WAL001", "") == "");
    CHECK(Transactions::addIncomeRecord(data, day, "SRC999", 100, "WAL001", "") == "");
    CHECK(Transactions::addIncomeRecord(data, day, "CAT001", 100, "WAL001", "") == "");  // a category id is not a source
    CHECK(Transactions::addIncomeRecord(data, day, "SRC001", 100, "", "") == "");
    CHECK(Transactions::addIncomeRecord(data, day, "SRC001", 100, "WAL999", "") == "");

    CHECK(Transactions::addExpenseRecord(data, day, "CAT001", 0, "WAL001", "") == "");
    CHECK(Transactions::addExpenseRecord(data, day, "CAT001", -100, "WAL001", "") == "");
    CHECK(Transactions::addExpenseRecord(data, Date(31, 4, 2026), "CAT001", 100, "WAL001", "") == "");
    CHECK(Transactions::addExpenseRecord(data, day, "", 100, "WAL001", "") == "");
    CHECK(Transactions::addExpenseRecord(data, day, "CAT999", 100, "WAL001", "") == "");
    CHECK(Transactions::addExpenseRecord(data, day, "SRC001", 100, "WAL001", "") == "");  // a source id is not a category
    CHECK(Transactions::addExpenseRecord(data, day, "CAT001", 100, "", "") == "");
    CHECK(Transactions::addExpenseRecord(data, day, "CAT001", 100, "WAL999", "") == "");

    CHECK(data.incomes.size() == incomesBefore);
    CHECK(data.expenses.size() == expensesBefore);
    CHECK(data.nextIncomeNo == nextIncomeBefore);
    CHECK(data.nextExpenseNo == nextExpenseBefore);

    // The next good transaction gets the next number (no gap).
    CHECK(Transactions::addIncomeRecord(data, day, "SRC001", 100, "WAL001", "") == "INC0003");
    CHECK(Transactions::addExpenseRecord(data, day, "CAT001", 100, "WAL001", "") == "EXP0003");

    // A deleted ID is never used again.
    Transactions::TransactionRef last;
    last.isIncome = true;
    last.index = data.incomes.size() - 1;
    CHECK(Transactions::deleteTransaction(data, last));
    CHECK(Transactions::addIncomeRecord(data, day, "SRC001", 100, "WAL001", "") == "INC0004");

    // The number comes from the counter (for example one loaded from a file).
    data.nextExpenseNo = 12;
    CHECK(Transactions::addExpenseRecord(data, day, "CAT001", 100, "WAL001", "") == "EXP0012");
    CHECK(data.nextExpenseNo == 13);
}

static Date crossRefDate(const FinanceData& data, const Transactions::TransactionRef& ref) {
    if (ref.isIncome) {
        return data.incomes[ref.index].getDate();
    }
    return data.expenses[ref.index].getDate();
}

// True if every income and every expense appears exactly once in the list.
static bool crossEveryOnce(const FinanceData& data, const DynamicArray<Transactions::TransactionRef>& refs) {
    if (refs.size() != data.incomes.size() + data.expenses.size()) {
        return false;
    }
    DynamicArray<int> seenIncome;
    DynamicArray<int> seenExpense;
    for (int i = 0; i < data.incomes.size(); i++) seenIncome.add(0);
    for (int i = 0; i < data.expenses.size(); i++) seenExpense.add(0);

    for (int i = 0; i < refs.size(); i++) {
        if (refs[i].isIncome) {
            if (refs[i].index < 0 || refs[i].index >= data.incomes.size()) return false;
            seenIncome.set(refs[i].index, seenIncome[refs[i].index] + 1);
        } else {
            if (refs[i].index < 0 || refs[i].index >= data.expenses.size()) return false;
            seenExpense.set(refs[i].index, seenExpense[refs[i].index] + 1);
        }
    }
    for (int i = 0; i < seenIncome.size(); i++) {
        if (seenIncome[i] != 1) return false;
    }
    for (int i = 0; i < seenExpense.size(); i++) {
        if (seenExpense[i] != 1) return false;
    }
    return true;
}

// True if no transaction is older than the one before it.
static bool crossNewestFirst(const FinanceData& data, const DynamicArray<Transactions::TransactionRef>& refs) {
    for (int i = 1; i < refs.size(); i++) {
        if (crossRefDate(data, refs[i - 1]) < crossRefDate(data, refs[i])) {
            return false;
        }
    }
    return true;
}

static void testCrossSortedOrder() {
    FinanceData data;
    data.sources.add(IncomeSource("SRC001", "Salary"));
    data.categories.add(ExpenseCategory("CAT001", "Food"));
    data.wallets.add(Wallet("WAL001", "Cash"));

    // Nothing to sort.
    CHECK(Transactions::getSortedTransactions(data).isEmpty());

    // Only incomes, added in a mixed order.
    data.incomes.add(Income("INC0001", Date(5, 3, 2026), "SRC001", 10, "WAL001", ""));
    data.incomes.add(Income("INC0002", Date(1, 3, 2026), "SRC001", 10, "WAL001", ""));
    data.incomes.add(Income("INC0003", Date(9, 3, 2026), "SRC001", 10, "WAL001", ""));
    DynamicArray<Transactions::TransactionRef> refs = Transactions::getSortedTransactions(data);
    CHECK(crossEveryOnce(data, refs));
    CHECK(crossNewestFirst(data, refs));
    if (refs.size() == 3) {
        CHECK(refs[0].index == 2);
        CHECK(refs[1].index == 0);
        CHECK(refs[2].index == 1);
    }

    // Only expenses.
    FinanceData onlyExpenses;
    onlyExpenses.expenses.add(Expense("EXP0001", Date(1, 1, 2026), "CAT001", 10, "WAL001", ""));
    onlyExpenses.expenses.add(Expense("EXP0002", Date(2, 1, 2026), "CAT001", 10, "WAL001", ""));
    refs = Transactions::getSortedTransactions(onlyExpenses);
    CHECK(crossEveryOnce(onlyExpenses, refs));
    if (refs.size() == 2) {
        CHECK(!refs[0].isIncome && refs[0].index == 1);
        CHECK(!refs[1].isIncome && refs[1].index == 0);
    }

    // Incomes and expenses mixed, across month and year borders.
    FinanceData mixed;
    mixed.expenses.add(Expense("EXP0001", Date(31, 12, 2025), "CAT001", 10, "WAL001", ""));
    mixed.incomes.add(Income("INC0001", Date(1, 1, 2026), "SRC001", 10, "WAL001", ""));
    mixed.expenses.add(Expense("EXP0002", Date(28, 2, 2026), "CAT001", 10, "WAL001", ""));
    mixed.incomes.add(Income("INC0002", Date(1, 3, 2026), "SRC001", 10, "WAL001", ""));
    mixed.expenses.add(Expense("EXP0003", Date(15, 6, 2024), "CAT001", 10, "WAL001", ""));
    refs = Transactions::getSortedTransactions(mixed);
    CHECK(crossEveryOnce(mixed, refs));
    if (refs.size() == 5) {
        CHECK(refs[0].isIncome && refs[0].index == 1);    // 01/03/2026
        CHECK(!refs[1].isIncome && refs[1].index == 1);   // 28/02/2026
        CHECK(refs[2].isIncome && refs[2].index == 0);    // 01/01/2026
        CHECK(!refs[3].isIncome && refs[3].index == 0);   // 31/12/2025
        CHECK(!refs[4].isIncome && refs[4].index == 2);   // 15/06/2024
    }

    // Same date: both types stay together between the newer and the older ones.
    FinanceData sameDay;
    Date tenth(10, 10, 2026);
    sameDay.incomes.add(Income("INC0001", tenth, "SRC001", 10, "WAL001", ""));
    sameDay.expenses.add(Expense("EXP0001", Date(9, 10, 2026), "CAT001", 10, "WAL001", ""));
    sameDay.expenses.add(Expense("EXP0002", tenth, "CAT001", 10, "WAL001", ""));
    sameDay.incomes.add(Income("INC0002", tenth, "SRC001", 10, "WAL001", ""));
    sameDay.incomes.add(Income("INC0003", Date(11, 10, 2026), "SRC001", 10, "WAL001", ""));
    sameDay.expenses.add(Expense("EXP0003", tenth, "CAT001", 10, "WAL001", ""));
    refs = Transactions::getSortedTransactions(sameDay);
    CHECK(crossEveryOnce(sameDay, refs));
    CHECK(crossNewestFirst(sameDay, refs));
    if (refs.size() == 6) {
        CHECK(crossRefDate(sameDay, refs[0]) == Date(11, 10, 2026));
        CHECK(crossRefDate(sameDay, refs[1]) == tenth);
        CHECK(crossRefDate(sameDay, refs[4]) == tenth);
        CHECK(crossRefDate(sameDay, refs[5]) == Date(9, 10, 2026));
    }

    // A longer list.
    FinanceData big;
    for (int i = 0; i < 60; i++) {
        Date date((i * 7) % 28 + 1, (i * 5) % 12 + 1, 2023 + i % 4);
        if (i % 2 == 0) {
            big.incomes.add(Income("INC", date, "SRC001", 10, "WAL001", ""));
        } else {
            big.expenses.add(Expense("EXP", date, "CAT001", 10, "WAL001", ""));
        }
    }
    refs = Transactions::getSortedTransactions(big);
    CHECK(refs.size() == 60);
    CHECK(crossEveryOnce(big, refs));
    CHECK(crossNewestFirst(big, refs));

    // The order follows new and deleted transactions.
    std::string newest = Transactions::addIncomeRecord(data, Date(1, 1, 2030), "SRC001", 10, "WAL001", "");
    refs = Transactions::getSortedTransactions(data);
    CHECK(crossEveryOnce(data, refs));
    CHECK(refs[0].isIncome && data.incomes[refs[0].index].getId() == newest);

    Transactions::TransactionRef top = refs[0];
    CHECK(Transactions::deleteTransaction(data, top));
    refs = Transactions::getSortedTransactions(data);
    CHECK(crossEveryOnce(data, refs));
    CHECK(crossNewestFirst(data, refs));
    CHECK(!(refs[0].isIncome && data.incomes[refs[0].index].getId() == newest));
}

static bool crossIncomeIsOriginal(const FinanceData& data) {
    const Income& a = data.incomes[0];
    const Income& b = data.incomes[1];
    return a.getId() == "INC0001" && a.getDate() == Date(1, 5, 2026) && a.getSourceId() == "SRC001" &&
           a.getAmount() == 100000 && a.getWalletId() == "WAL001" && a.getDescription() == "Old" &&
           a.getRecurringId() == "REC001" &&
           b.getId() == "INC0002" && b.getDate() == Date(2, 5, 2026) && b.getSourceId() == "SRC002" &&
           b.getAmount() == 200000 && b.getWalletId() == "WAL002" && b.getDescription() == "Other";
}

static bool crossExpenseIsOriginal(const FinanceData& data) {
    const Expense& a = data.expenses[0];
    const Expense& b = data.expenses[1];
    return a.getId() == "EXP0001" && a.getDate() == Date(3, 5, 2026) && a.getCategoryId() == "CAT001" &&
           a.getAmount() == 50000 && a.getWalletId() == "WAL001" && a.getDescription() == "Old" &&
           a.getRecurringId() == "REC002" &&
           b.getId() == "EXP0002" && b.getDate() == Date(4, 5, 2026) && b.getCategoryId() == "CAT002" &&
           b.getAmount() == 60000 && b.getWalletId() == "WAL002" && b.getDescription() == "Other";
}

static void testCrossUpdateTransactions() {
    FinanceData data;
    data.sources.add(IncomeSource("SRC001", "Salary"));
    data.sources.add(IncomeSource("SRC002", "Bonus"));
    data.categories.add(ExpenseCategory("CAT001", "Food"));
    data.categories.add(ExpenseCategory("CAT002", "Transport"));
    data.wallets.add(Wallet("WAL001", "Cash"));
    data.wallets.add(Wallet("WAL002", "Bank"));
    data.incomes.add(Income("INC0001", Date(1, 5, 2026), "SRC001", 100000, "WAL001", "Old", "REC001"));
    data.incomes.add(Income("INC0002", Date(2, 5, 2026), "SRC002", 200000, "WAL002", "Other"));
    data.expenses.add(Expense("EXP0001", Date(3, 5, 2026), "CAT001", 50000, "WAL001", "Old", "REC002"));
    data.expenses.add(Expense("EXP0002", Date(4, 5, 2026), "CAT002", 60000, "WAL002", "Other"));

    // ----- Income: a bad update is refused and changes nothing -----
    Income good = data.incomes[0];
    Income bad = good;

    bad = good; bad.setAmount(0);
    CHECK(!Transactions::updateIncome(data, 0, bad));
    bad = good; bad.setAmount(-1);
    CHECK(!Transactions::updateIncome(data, 0, bad));
    bad = good; bad.setDate(Date(30, 2, 2026));
    CHECK(!Transactions::updateIncome(data, 0, bad));
    bad = good; bad.setSourceId("");
    CHECK(!Transactions::updateIncome(data, 0, bad));
    bad = good; bad.setSourceId("SRC999");
    CHECK(!Transactions::updateIncome(data, 0, bad));
    bad = good; bad.setSourceId("CAT001");
    CHECK(!Transactions::updateIncome(data, 0, bad));
    bad = good; bad.setWalletId("");
    CHECK(!Transactions::updateIncome(data, 0, bad));
    bad = good; bad.setWalletId("WAL999");
    CHECK(!Transactions::updateIncome(data, 0, bad));
    CHECK(!Transactions::updateIncome(data, -1, good));
    CHECK(!Transactions::updateIncome(data, 2, good));
    CHECK(crossIncomeIsOriginal(data));

    // ----- Income: good updates change only that income -----
    Income changed = good;
    changed.setAmount(150000);
    CHECK(Transactions::updateIncome(data, 0, changed));
    CHECK(data.incomes[0].getAmount() == 150000);
    CHECK(data.incomes[0].getId() == "INC0001");
    CHECK(data.incomes[0].getWalletId() == "WAL001");
    CHECK(data.incomes[0].getDescription() == "Old");
    CHECK(data.incomes[1].getAmount() == 200000);

    changed = data.incomes[0];
    changed.setWalletId("WAL002");
    changed.setSourceId("SRC002");
    changed.setDate(Date(20, 5, 2026));
    changed.setDescription("New");
    CHECK(Transactions::updateIncome(data, 0, changed));
    CHECK(data.incomes[0].getWalletId() == "WAL002");
    CHECK(data.incomes[0].getSourceId() == "SRC002");
    CHECK(data.incomes[0].getDate() == Date(20, 5, 2026));
    CHECK(data.incomes[0].getDescription() == "New");
    CHECK(data.incomes[0].getAmount() == 150000);
    CHECK(data.incomes[0].getRecurringId() == "REC001");
    CHECK(data.incomes[1].getId() == "INC0002");
    CHECK(data.incomes[1].getAmount() == 200000);

    // Update the second income: the first one stays as it is.
    Income second = data.incomes[1];
    second.setAmount(1);
    CHECK(Transactions::updateIncome(data, 1, second));
    CHECK(data.incomes[1].getAmount() == 1);
    CHECK(data.incomes[0].getAmount() == 150000);

    // ----- Expense: a bad update is refused and changes nothing -----
    Expense goodExpense = data.expenses[0];
    Expense badExpense = goodExpense;

    badExpense = goodExpense; badExpense.setAmount(0);
    CHECK(!Transactions::updateExpense(data, 0, badExpense));
    badExpense = goodExpense; badExpense.setAmount(-1);
    CHECK(!Transactions::updateExpense(data, 0, badExpense));
    badExpense = goodExpense; badExpense.setDate(Date(30, 2, 2026));
    CHECK(!Transactions::updateExpense(data, 0, badExpense));
    badExpense = goodExpense; badExpense.setCategoryId("");
    CHECK(!Transactions::updateExpense(data, 0, badExpense));
    badExpense = goodExpense; badExpense.setCategoryId("CAT999");
    CHECK(!Transactions::updateExpense(data, 0, badExpense));
    badExpense = goodExpense; badExpense.setCategoryId("SRC001");
    CHECK(!Transactions::updateExpense(data, 0, badExpense));
    badExpense = goodExpense; badExpense.setWalletId("");
    CHECK(!Transactions::updateExpense(data, 0, badExpense));
    badExpense = goodExpense; badExpense.setWalletId("WAL999");
    CHECK(!Transactions::updateExpense(data, 0, badExpense));
    CHECK(!Transactions::updateExpense(data, -1, goodExpense));
    CHECK(!Transactions::updateExpense(data, 2, goodExpense));
    CHECK(crossExpenseIsOriginal(data));

    // ----- Expense: good updates change only that expense -----
    Expense changedExpense = goodExpense;
    changedExpense.setCategoryId("CAT002");
    changedExpense.setWalletId("WAL002");
    changedExpense.setAmount(75000);
    changedExpense.setDescription("New");
    CHECK(Transactions::updateExpense(data, 0, changedExpense));
    CHECK(data.expenses[0].getCategoryId() == "CAT002");
    CHECK(data.expenses[0].getWalletId() == "WAL002");
    CHECK(data.expenses[0].getAmount() == 75000);
    CHECK(data.expenses[0].getDescription() == "New");
    CHECK(data.expenses[0].getId() == "EXP0001");
    CHECK(data.expenses[0].getRecurringId() == "REC002");
    CHECK(data.expenses[1].getId() == "EXP0002");
    CHECK(data.expenses[1].getAmount() == 60000);

    // A new date moves the transaction in the sorted list.
    changedExpense = data.expenses[0];
    changedExpense.setDate(Date(1, 1, 2030));
    CHECK(Transactions::updateExpense(data, 0, changedExpense));
    DynamicArray<Transactions::TransactionRef> refs = Transactions::getSortedTransactions(data);
    CHECK(refs.size() == 4);
    CHECK(!refs[0].isIncome && refs[0].index == 0);
}

static void testCrossDeleteTransactions() {
    FinanceData data;
    data.sources.add(IncomeSource("SRC001", "Salary"));
    data.categories.add(ExpenseCategory("CAT001", "Food"));
    data.wallets.add(Wallet("WAL001", "Cash"));
    Date day(10, 11, 2026);
    for (int i = 1; i <= 5; i++) {
        Transactions::addIncomeRecord(data, day, "SRC001", 1000 * i, "WAL001", "");
    }
    for (int i = 1; i <= 3; i++) {
        Transactions::addExpenseRecord(data, day, "CAT001", 100 * i, "WAL001", "");
    }
    CHECK(data.incomes.size() == 5);
    CHECK(data.expenses.size() == 3);

    Transactions::TransactionRef ref;

    // A wrong index changes nothing.
    ref.isIncome = true;  ref.index = -1;
    CHECK(!Transactions::deleteTransaction(data, ref));
    ref.isIncome = true;  ref.index = 5;
    CHECK(!Transactions::deleteTransaction(data, ref));
    ref.isIncome = false; ref.index = -1;
    CHECK(!Transactions::deleteTransaction(data, ref));
    ref.isIncome = false; ref.index = 3;
    CHECK(!Transactions::deleteTransaction(data, ref));
    CHECK(data.incomes.size() == 5);
    CHECK(data.expenses.size() == 3);

    // Deleting an income does not touch the expenses.
    ref.isIncome = true;  ref.index = 2;
    CHECK(Transactions::deleteTransaction(data, ref));
    CHECK(data.incomes.size() == 4);
    CHECK(data.expenses.size() == 3);
    CHECK(data.incomes[0].getId() == "INC0001");
    CHECK(data.incomes[1].getId() == "INC0002");
    CHECK(data.incomes[2].getId() == "INC0004");
    CHECK(data.incomes[3].getId() == "INC0005");

    // Deleting an expense does not touch the incomes.
    ref.isIncome = false; ref.index = 0;
    CHECK(Transactions::deleteTransaction(data, ref));
    CHECK(data.expenses.size() == 2);
    CHECK(data.incomes.size() == 4);
    CHECK(data.expenses[0].getId() == "EXP0002");
    CHECK(data.expenses[1].getId() == "EXP0003");

    // Delete the last income: the old last index is wrong now.
    ref.isIncome = true;  ref.index = 3;
    CHECK(Transactions::deleteTransaction(data, ref));
    CHECK(data.incomes.size() == 3);
    CHECK(data.incomes[2].getId() == "INC0004");
    CHECK(!Transactions::deleteTransaction(data, ref));
    CHECK(data.incomes.size() == 3);

    // The sorted list only contains what is left.
    DynamicArray<Transactions::TransactionRef> refs = Transactions::getSortedTransactions(data);
    CHECK(refs.size() == 5);

    // Delete everything: the lists are empty and the ID counters keep counting.
    while (!data.incomes.isEmpty()) {
        ref.isIncome = true;  ref.index = 0;
        CHECK(Transactions::deleteTransaction(data, ref));
    }
    while (!data.expenses.isEmpty()) {
        ref.isIncome = false; ref.index = 0;
        CHECK(Transactions::deleteTransaction(data, ref));
    }
    CHECK(Transactions::getSortedTransactions(data).isEmpty());
    CHECK(Transactions::addIncomeRecord(data, day, "SRC001", 1000, "WAL001", "") == "INC0006");
    CHECK(Transactions::addExpenseRecord(data, day, "CAT001", 100, "WAL001", "") == "EXP0004");
}

static void testCrossTransactionsWithOtherModules() {
    FinanceData data;
    std::string message;
    MasterData::addItem(data, MasterData::Kind::WALLET, "Cash", message);
    MasterData::addItem(data, MasterData::Kind::WALLET, "Bank", message);
    MasterData::addItem(data, MasterData::Kind::WALLET, "Spare", message);
    MasterData::addItem(data, MasterData::Kind::SOURCE, "Salary", message);
    MasterData::addItem(data, MasterData::Kind::CATEGORY, "Food", message);
    MasterData::addItem(data, MasterData::Kind::CATEGORY, "Rent", message);
    Date day(10, 11, 2026);

    // Nothing uses the master data yet.
    CHECK(!MasterData::isWalletInUse(data, "WAL001"));
    CHECK(!MasterData::isSourceInUse(data, "SRC001"));
    CHECK(!MasterData::isCategoryInUse(data, "CAT001"));

    Transactions::addIncomeRecord(data, day, "SRC001", 1000000, "WAL001", "");   // INC0001
    Transactions::addExpenseRecord(data, day, "CAT001", 300000, "WAL001", "");   // EXP0001
    Transactions::addIncomeRecord(data, day, "SRC001", 500000, "WAL002", "");    // INC0002

    // The balances follow the new transactions.
    CHECK(Balance::walletBalance(data, "WAL001") == 700000);
    CHECK(Balance::walletBalance(data, "WAL002") == 500000);
    CHECK(Balance::walletBalance(data, "WAL003") == 0);
    CHECK(Balance::totalBalance(data) == 1200000);

    // Names and usage are found through the IDs.
    CHECK(MasterData::getWalletName(data, data.incomes[0].getWalletId()) == "Cash");
    CHECK(MasterData::getSourceName(data, data.incomes[0].getSourceId()) == "Salary");
    CHECK(MasterData::getCategoryName(data, data.expenses[0].getCategoryId()) == "Food");
    CHECK(MasterData::isWalletInUse(data, "WAL001"));
    CHECK(MasterData::isWalletInUse(data, "WAL002"));
    CHECK(!MasterData::isWalletInUse(data, "WAL003"));
    CHECK(MasterData::isSourceInUse(data, "SRC001"));
    CHECK(MasterData::isCategoryInUse(data, "CAT001"));
    CHECK(!MasterData::isCategoryInUse(data, "CAT002"));

    // Moving the expense to another wallet moves its money too.
    Expense moved = data.expenses[0];
    moved.setWalletId("WAL002");
    CHECK(Transactions::updateExpense(data, 0, moved));
    CHECK(Balance::walletBalance(data, "WAL001") == 1000000);
    CHECK(Balance::walletBalance(data, "WAL002") == 200000);
    CHECK(Balance::totalBalance(data) == 1200000);

    // Deleting an income lowers the balance; a wallet may go below zero.
    Transactions::TransactionRef ref;
    ref.isIncome = true;
    ref.index = 1;
    CHECK(Transactions::deleteTransaction(data, ref));
    CHECK(Balance::walletBalance(data, "WAL002") == -300000);
    CHECK(Balance::totalBalance(data) == 700000);

    // A wallet used by a transaction cannot be removed.
    CHECK(!MasterData::removeItem(data, MasterData::Kind::WALLET, "WAL002", message));
    CHECK(message != "");
    CHECK(data.wallets.size() == 3);

    // Once the last transaction using it is deleted, it can be removed.
    ref.isIncome = false;
    ref.index = 0;
    CHECK(Transactions::deleteTransaction(data, ref));
    CHECK(!MasterData::isWalletInUse(data, "WAL002"));
    CHECK(!MasterData::isCategoryInUse(data, "CAT001"));
    CHECK(MasterData::removeItem(data, MasterData::Kind::WALLET, "WAL002", message));
    CHECK(MasterData::removeItem(data, MasterData::Kind::CATEGORY, "CAT001", message));
    CHECK(message == "");
    CHECK(data.wallets.size() == 2);
    CHECK(Balance::totalBalance(data) == 1000000);
}

static void testAuth() {
    DynamicArray<User> users;
    std::string message;

    // Register: every rule of the form.
    CHECK(!Auth::registerUser(users, "khoa", "   ", "secret1", "secret1", message) && message != "");
    CHECK(!Auth::registerUser(users, "khoa", "A very long nickname here", "secret1", "secret1", message));
    CHECK(!Auth::registerUser(users, "kh", "Khoa", "secret1", "secret1", message));
    CHECK(!Auth::registerUser(users, "khoa nguyen", "Khoa", "secret1", "secret1", message));
    CHECK(!Auth::registerUser(users, "khoa", "Khoa", "12345", "12345", message));
    CHECK(!Auth::registerUser(users, "khoa", "Khoa", "secret1", "secret2", message));
    CHECK(users.isEmpty());
    CHECK(Auth::registerUser(users, "  Khoa_23 ", " Khoa ", "secret1", "secret1", message) && message == "");
    CHECK(users.size() == 1);
    CHECK(users[0].getUsername() == "Khoa_23" && users[0].getNickname() == "Khoa");
    // The password itself is never stored.
    CHECK(users[0].getPasswordHash() != "secret1" && users[0].getPasswordHash().size() == 16);
    // Username already taken (case ignored).
    CHECK(!Auth::registerUser(users, "khoa_23", "Other", "secret9", "secret9", message));
    CHECK(Auth::registerUser(users, "quan", "Quan", "abcdef", "abcdef", message));

    // Same password, different salt -> different hash.
    CHECK(users[0].getSalt() != users[1].getSalt());
    CHECK(Auth::hashPassword("abc123", "s1") == Auth::hashPassword("abc123", "s1"));
    CHECK(Auth::hashPassword("abc123", "s1") != Auth::hashPassword("abc123", "s2"));
    CHECK(Auth::hashPassword("abc123", "s1") != Auth::hashPassword("abc124", "s1"));

    // Log in
    CHECK(Auth::login(users, "khoa_23", "secret1", message) == 0 && message == "");
    CHECK(Auth::login(users, "QUAN", "abcdef", message) == 1);
    CHECK(Auth::login(users, "quan", "secret1", message) == -1 && message == "Wrong username or password.");
    CHECK(Auth::login(users, "nobody", "abcdef", message) == -1 && message == "Wrong username or password.");
    CHECK(Auth::login(users, "", "", message) == -1 && message != "");

    // Forgot password: the nickname must match (case and spaces ignored).
    CHECK(!Auth::resetPassword(users, "quan", "Khoa", "newpass1", "newpass1", message));
    CHECK(message == "Username and nickname do not match.");
    CHECK(!Auth::resetPassword(users, "nobody", "Quan", "newpass1", "newpass1", message));
    CHECK(message == "Username and nickname do not match.");
    CHECK(!Auth::resetPassword(users, "quan", "Quan", "short", "short", message));
    CHECK(!Auth::resetPassword(users, "quan", "Quan", "newpass1", "newpass2", message));
    CHECK(!Auth::resetPassword(users, "", "", "newpass1", "newpass1", message) && message != "");
    CHECK(Auth::login(users, "quan", "abcdef", message) == 1);   // nothing changed yet
    std::string oldSalt = users[1].getSalt();
    CHECK(Auth::resetPassword(users, " QUAN ", "  quan ", "newpass1", "newpass1", message) && message == "");
    CHECK(users[1].getSalt() != oldSalt);
    CHECK(Auth::login(users, "quan", "abcdef", message) == -1);
    CHECK(Auth::login(users, "quan", "newpass1", message) == 1);
    CHECK(Auth::login(users, "khoa_23", "secret1", message) == 0);   // other account untouched

    // Change password: the current password must be right.
    CHECK(!Auth::changePassword(users, 0, "wrong!", "secret2", "secret2", message));
    CHECK(message == "The current password is wrong.");
    CHECK(!Auth::changePassword(users, 0, "secret1", "secret1", "secret1", message));
    CHECK(!Auth::changePassword(users, 0, "secret1", "abc", "abc", message));
    CHECK(!Auth::changePassword(users, 0, "secret1", "secret2", "secret3", message));
    CHECK(!Auth::changePassword(users, 5, "secret1", "secret2", "secret2", message));
    CHECK(Auth::login(users, "khoa_23", "secret1", message) == 0);   // still the old one
    CHECK(Auth::changePassword(users, 0, "secret1", "secret2", "secret2", message) && message == "");
    CHECK(Auth::login(users, "khoa_23", "secret1", message) == -1);
    CHECK(Auth::login(users, "khoa_23", "secret2", message) == 0);
    CHECK(Auth::changePassword(users, 0, "secret2", "secret1", "secret1", message));   // back for the tests below

    // Each account has its own data file.
    CHECK(Auth::dataFileFor("data/users", "Khoa_23") == "data/users/khoa_23.dat");
    CHECK(Auth::dataFileFor("data/users", "quan") != Auth::dataFileFor("data/users", "khoa_23"));

    // Save and load the account list.
    const std::string path = "test_users.dat";
    CHECK(Auth::saveUsers(users, path));
    DynamicArray<User> loaded;
    CHECK(Auth::loadUsers(loaded, path));
    CHECK(loaded.size() == 2);
    CHECK(loaded[0].getUsername() == "Khoa_23" && loaded[0].getNickname() == "Khoa");
    CHECK(loaded[1].getPasswordHash() == users[1].getPasswordHash() && loaded[1].getSalt() == users[1].getSalt());
    CHECK(Auth::login(loaded, "khoa_23", "secret1", message) == 0);

    // Missing or damaged file: false, empty list, no crash.
    std::remove(path.c_str());
    CHECK(!Auth::loadUsers(loaded, path) && loaded.isEmpty());
    {
        std::ofstream out(path, std::ios::binary);
        out << "not a users file";
    }
    CHECK(!Auth::loadUsers(loaded, path) && loaded.isEmpty());
    std::remove(path.c_str());
}

static void testCrossRecurringCrud() {
    FinanceData data;
    data.sources.add(IncomeSource("SRC001", "Salary"));
    data.sources.add(IncomeSource("SRC002", "Freelance"));
    data.categories.add(ExpenseCategory("CAT001", "Rent"));
    data.categories.add(ExpenseCategory("CAT002", "Internet"));
    data.wallets.add(Wallet("WAL001", "Cash"));
    data.wallets.add(Wallet("WAL002", "Bank"));

    Date validStart(15, 6, 2026);
    Date validEnd(15, 12, 2026);
    Date earlierEnd(14, 6, 2026);
    Date invalidDate(30, 2, 2026);

    CHECK(Recurring::addRecurring(data, RecurringTransaction("", TransactionType::INCOME, "SRC001", 0LL, "WAL001", "", validStart, false, Date())) == "");
    CHECK(Recurring::addRecurring(data, RecurringTransaction("", TransactionType::INCOME, "SRC001", -1LL, "WAL001", "", validStart, false, Date())) == "");
    CHECK(Recurring::addRecurring(data, RecurringTransaction("", TransactionType::EXPENSE, "CAT001", -5000000000LL, "WAL001", "", validStart, false, Date())) == "");

    CHECK(Recurring::addRecurring(data, RecurringTransaction("", TransactionType::INCOME, "SRC001", 1000LL, "WAL001", "", invalidDate, false, Date())) == "");
    CHECK(Recurring::addRecurring(data, RecurringTransaction("", TransactionType::INCOME, "SRC001", 1000LL, "WAL001", "", Date(29, 2, 2025), false, Date())) == "");
    CHECK(Recurring::addRecurring(data, RecurringTransaction("", TransactionType::EXPENSE, "CAT001", 1000LL, "WAL001", "", validStart, true, invalidDate)) == "");
    CHECK(Recurring::addRecurring(data, RecurringTransaction("", TransactionType::EXPENSE, "CAT001", 1000LL, "WAL001", "", validStart, true, earlierEnd)) == "");

    CHECK(Recurring::addRecurring(data, RecurringTransaction("", TransactionType::INCOME, "CAT001", 1000LL, "WAL001", "", validStart, false, Date())) == "");
    CHECK(Recurring::addRecurring(data, RecurringTransaction("", TransactionType::EXPENSE, "SRC001", 1000LL, "WAL001", "", validStart, false, Date())) == "");
    CHECK(Recurring::addRecurring(data, RecurringTransaction("", TransactionType::INCOME, "SRC999", 1000LL, "WAL001", "", validStart, false, Date())) == "");
    CHECK(Recurring::addRecurring(data, RecurringTransaction("", TransactionType::EXPENSE, "CAT999", 1000LL, "WAL001", "", validStart, false, Date())) == "");
    CHECK(Recurring::addRecurring(data, RecurringTransaction("", TransactionType::INCOME, "SRC001", 1000LL, "WAL999", "", validStart, false, Date())) == "");
    CHECK(Recurring::addRecurring(data, RecurringTransaction("", TransactionType::INCOME, "", 1000LL, "WAL001", "", validStart, false, Date())) == "");
    CHECK(Recurring::addRecurring(data, RecurringTransaction("", TransactionType::EXPENSE, "", 1000LL, "WAL001", "", validStart, false, Date())) == "");
    CHECK(Recurring::addRecurring(data, RecurringTransaction("", TransactionType::INCOME, "SRC001", 1000LL, "", "", validStart, false, Date())) == "");

    CHECK(data.recurrings.isEmpty());
    CHECK(data.nextRecurringNo == 1);

    CHECK(Recurring::addRecurring(data, RecurringTransaction("CUSTOM1", TransactionType::INCOME, "SRC001", 5000000000LL, "WAL001", "Big salary", validStart, false, invalidDate)) == "REC001");
    CHECK(Recurring::addRecurring(data, RecurringTransaction("CUSTOM2", TransactionType::EXPENSE, "CAT001", 2500000LL, "WAL002", "Rent", validStart, false, earlierEnd)) == "REC002");
    CHECK(Recurring::addRecurring(data, RecurringTransaction("CUSTOM3", TransactionType::EXPENSE, "CAT002", 300000LL, "WAL001", "Net", validStart, true, validStart)) == "REC003");
    CHECK(Recurring::addRecurring(data, RecurringTransaction("CUSTOM4", TransactionType::INCOME, "SRC002", 1500000LL, "WAL002", "Side", validStart, true, validEnd)) == "REC004");
    CHECK(data.recurrings.size() == 4);
    CHECK(data.nextRecurringNo == 5);
    CHECK(data.recurrings[0].getId() == "REC001");
    CHECK(data.recurrings[0].getAmount() == 5000000000LL);
    CHECK(!data.recurrings[0].getHasEndDate());
    CHECK(data.recurrings[1].getId() == "REC002");
    CHECK(!data.recurrings[1].getHasEndDate());
    CHECK(data.recurrings[2].getId() == "REC003");
    CHECK(data.recurrings[2].getHasEndDate() && data.recurrings[2].getEndDate() == validStart);
    CHECK(data.recurrings[3].getId() == "REC004");

    RecurringTransaction validUpdate("IGNORED_ID", TransactionType::EXPENSE, "CAT002", 7000000000LL, "WAL002", "Updated", Date(1, 7, 2026), true, Date(31, 12, 2026));
    CHECK(!Recurring::updateRecurring(data, -1, validUpdate));
    CHECK(!Recurring::updateRecurring(data, data.recurrings.size(), validUpdate));
    CHECK(!Recurring::updateRecurring(data, 100, validUpdate));
    CHECK(!Recurring::updateRecurring(data, 0, RecurringTransaction("X", TransactionType::INCOME, "SRC001", 0LL, "WAL001", "", validStart, false, Date())));
    CHECK(!Recurring::updateRecurring(data, 0, RecurringTransaction("X", TransactionType::INCOME, "SRC001", -100LL, "WAL001", "", validStart, false, Date())));
    CHECK(!Recurring::updateRecurring(data, 0, RecurringTransaction("X", TransactionType::INCOME, "SRC001", 100LL, "WAL001", "", invalidDate, false, Date())));
    CHECK(!Recurring::updateRecurring(data, 0, RecurringTransaction("X", TransactionType::INCOME, "SRC001", 100LL, "WAL001", "", validStart, true, invalidDate)));
    CHECK(!Recurring::updateRecurring(data, 0, RecurringTransaction("X", TransactionType::INCOME, "SRC001", 100LL, "WAL001", "", validStart, true, earlierEnd)));
    CHECK(!Recurring::updateRecurring(data, 0, RecurringTransaction("X", TransactionType::INCOME, "CAT001", 100LL, "WAL001", "", validStart, false, Date())));
    CHECK(!Recurring::updateRecurring(data, 0, RecurringTransaction("X", TransactionType::EXPENSE, "SRC001", 100LL, "WAL001", "", validStart, false, Date())));
    CHECK(!Recurring::updateRecurring(data, 0, RecurringTransaction("X", TransactionType::INCOME, "SRC001", 100LL, "WAL999", "", validStart, false, Date())));
    CHECK(!Recurring::updateRecurring(data, 0, RecurringTransaction("X", TransactionType::INCOME, "", 100LL, "WAL001", "", validStart, false, Date())));
    CHECK(!Recurring::updateRecurring(data, 0, RecurringTransaction("X", TransactionType::INCOME, "SRC001", 100LL, "", "", validStart, false, Date())));

    CHECK(data.recurrings.size() == 4);
    CHECK(data.nextRecurringNo == 5);
    CHECK(data.recurrings[0].getId() == "REC001");
    CHECK(data.recurrings[0].getType() == TransactionType::INCOME);
    CHECK(data.recurrings[0].getRefId() == "SRC001");
    CHECK(data.recurrings[0].getAmount() == 5000000000LL);
    CHECK(data.recurrings[0].getWalletId() == "WAL001");
    CHECK(data.recurrings[0].getDescription() == "Big salary");

    CHECK(Recurring::updateRecurring(data, 0, validUpdate));
    CHECK(data.recurrings[0].getId() == "REC001");
    CHECK(data.recurrings[0].getType() == TransactionType::EXPENSE);
    CHECK(data.recurrings[0].getRefId() == "CAT002");
    CHECK(data.recurrings[0].getAmount() == 7000000000LL);
    CHECK(data.recurrings[0].getWalletId() == "WAL002");
    CHECK(data.recurrings[0].getDescription() == "Updated");
    CHECK(data.recurrings[0].getStartDate() == Date(1, 7, 2026));
    CHECK(data.recurrings[0].getHasEndDate());
    CHECK(data.recurrings[0].getEndDate() == Date(31, 12, 2026));
    CHECK(data.nextRecurringNo == 5);

    RecurringTransaction updateIgnoreEnd("OTHER", TransactionType::INCOME, "SRC002", 400000LL, "WAL001", "No end", validStart, false, invalidDate);
    CHECK(Recurring::updateRecurring(data, 2, updateIgnoreEnd));
    CHECK(data.recurrings[2].getId() == "REC003");
    CHECK(!data.recurrings[2].getHasEndDate());

    CHECK(!Recurring::removeRecurring(data, -1));
    CHECK(!Recurring::removeRecurring(data, data.recurrings.size()));
    CHECK(!Recurring::removeRecurring(data, 99));
    CHECK(data.recurrings.size() == 4);
    CHECK(data.nextRecurringNo == 5);

    CHECK(Recurring::removeRecurring(data, 1));
    CHECK(data.recurrings.size() == 3);
    CHECK(data.recurrings[0].getId() == "REC001");
    CHECK(data.recurrings[1].getId() == "REC003");
    CHECK(data.recurrings[2].getId() == "REC004");

    CHECK(Recurring::removeRecurring(data, 0));
    CHECK(data.recurrings.size() == 2);
    CHECK(data.recurrings[0].getId() == "REC003");
    CHECK(data.recurrings[1].getId() == "REC004");

    CHECK(Recurring::removeRecurring(data, data.recurrings.size() - 1));
    CHECK(data.recurrings.size() == 1);
    CHECK(data.recurrings[0].getId() == "REC003");

    CHECK(Recurring::removeRecurring(data, 0));
    CHECK(data.recurrings.isEmpty());
    CHECK(!Recurring::removeRecurring(data, 0));
    CHECK(data.nextRecurringNo == 5);

    CHECK(Recurring::addRecurring(data, RecurringTransaction("", TransactionType::INCOME, "SRC001", 100000LL, "WAL001", "New", validStart, false, Date())) == "REC005");
    CHECK(data.recurrings.size() == 1);
    CHECK(data.recurrings[0].getId() == "REC005");
    CHECK(data.nextRecurringNo == 6);
}

static void testCrossGenerateDueTransactions() {
    FinanceData data;
    data.sources.add(IncomeSource("SRC001", "Salary"));
    data.categories.add(ExpenseCategory("CAT001", "Rent"));
    data.categories.add(ExpenseCategory("CAT002", "Gym"));
    data.wallets.add(Wallet("WAL001", "Bank"));

    CHECK(Recurring::addRecurring(data, RecurringTransaction("", TransactionType::EXPENSE, "CAT001", 2000000LL, "WAL001", "Rent", Date(15, 3, 2026), true, Date(10, 5, 2026))) == "REC001");

    CHECK(Recurring::generateDueTransactions(data, Date(28, 2, 2026)) == 0);
    CHECK(data.expenses.isEmpty());
    CHECK(data.nextExpenseNo == 1);

    CHECK(Recurring::generateDueTransactions(data, Date(1, 3, 2026)) == 1);
    CHECK(data.expenses.size() == 1);
    CHECK(data.expenses[0].getId() == "EXP0001");
    CHECK(data.expenses[0].getDate() == Date(15, 3, 2026));
    CHECK(data.expenses[0].getAmount() == 2000000LL);
    CHECK(data.expenses[0].getRecurringId() == "REC001");
    CHECK(data.nextExpenseNo == 2);

    CHECK(Recurring::generateDueTransactions(data, Date(15, 3, 2026)) == 0);
    CHECK(Recurring::generateDueTransactions(data, Date(31, 3, 2026)) == 0);
    CHECK(data.expenses.size() == 1);
    CHECK(data.nextExpenseNo == 2);

    CHECK(Recurring::generateDueTransactions(data, Date(31, 5, 2026)) == 1);
    CHECK(data.expenses.size() == 2);
    CHECK(data.expenses[1].getDate() == Date(15, 5, 2026));

    CHECK(Recurring::generateDueTransactions(data, Date(1, 6, 2026)) == 0);
    CHECK(data.expenses.size() == 2);

    FinanceData clampData;
    clampData.sources.add(IncomeSource("SRC001", "Pay"));
    clampData.categories.add(ExpenseCategory("CAT001", "Bill"));
    clampData.wallets.add(Wallet("WAL001", "Cash"));
    CHECK(Recurring::addRecurring(clampData, RecurringTransaction("", TransactionType::INCOME, "SRC001", 3000000000LL, "WAL001", "Day31", Date(31, 1, 2026), false, Date())) == "REC001");
    CHECK(Recurring::addRecurring(clampData, RecurringTransaction("", TransactionType::EXPENSE, "CAT001", 500000LL, "WAL001", "Day30", Date(30, 1, 2026), false, Date())) == "REC002");

    CHECK(Recurring::generateDueTransactions(clampData, Date(5, 4, 2026)) == 2);
    CHECK(clampData.incomes[0].getDate() == Date(30, 4, 2026));
    CHECK(clampData.expenses[0].getDate() == Date(30, 4, 2026));

    CHECK(Recurring::generateDueTransactions(clampData, Date(10, 6, 2026)) == 2);
    CHECK(clampData.incomes[1].getDate() == Date(30, 6, 2026));
    CHECK(clampData.expenses[1].getDate() == Date(30, 6, 2026));

    CHECK(Recurring::generateDueTransactions(clampData, Date(1, 2, 2026)) == 2);
    CHECK(clampData.incomes[2].getDate() == Date(28, 2, 2026));
    CHECK(clampData.expenses[2].getDate() == Date(28, 2, 2026));

    CHECK(Recurring::generateDueTransactions(clampData, Date(1, 2, 2028)) == 2);
    CHECK(clampData.incomes[3].getDate() == Date(29, 2, 2028));
    CHECK(clampData.expenses[3].getDate() == Date(29, 2, 2028));

    FinanceData sameMonthData;
    sameMonthData.categories.add(ExpenseCategory("CAT001", "Event"));
    sameMonthData.wallets.add(Wallet("WAL001", "Cash"));
    CHECK(Recurring::addRecurring(sameMonthData, RecurringTransaction("", TransactionType::EXPENSE, "CAT001", 750000LL, "WAL001", "OneMonth", Date(10, 5, 2026), true, Date(20, 5, 2026))) == "REC001");

    CHECK(Recurring::generateDueTransactions(sameMonthData, Date(30, 4, 2026)) == 0);
    CHECK(Recurring::generateDueTransactions(sameMonthData, Date(1, 5, 2026)) == 1);
    CHECK(sameMonthData.expenses.size() == 1);
    CHECK(sameMonthData.expenses[0].getDate() == Date(10, 5, 2026));
    CHECK(Recurring::generateDueTransactions(sameMonthData, Date(25, 5, 2026)) == 0);
    CHECK(Recurring::generateDueTransactions(sameMonthData, Date(1, 6, 2026)) == 0);
    CHECK(sameMonthData.expenses.size() == 1);

    FinanceData yearData;
    yearData.sources.add(IncomeSource("SRC001", "Bonus"));
    yearData.wallets.add(Wallet("WAL001", "Bank"));
    CHECK(Recurring::addRecurring(yearData, RecurringTransaction("", TransactionType::INCOME, "SRC001", 1000000LL, "WAL001", "YearlyCheck", Date(12, 3, 2026), false, Date())) == "REC001");

    CHECK(Recurring::generateDueTransactions(yearData, Date(1, 3, 2026)) == 1);
    CHECK(Recurring::generateDueTransactions(yearData, Date(1, 3, 2027)) == 1);
    CHECK(Recurring::generateDueTransactions(yearData, Date(1, 3, 2028)) == 1);
    CHECK(yearData.incomes.size() == 3);
    CHECK(yearData.incomes[0].getDate() == Date(12, 3, 2026));
    CHECK(yearData.incomes[1].getDate() == Date(12, 3, 2027));
    CHECK(yearData.incomes[2].getDate() == Date(12, 3, 2028));

    int prevIncomes = yearData.incomes.size();
    int prevExpenses = yearData.expenses.size();
    int prevNextInc = yearData.nextIncomeNo;
    int prevNextExp = yearData.nextExpenseNo;
    CHECK(Recurring::generateDueTransactions(yearData, Date(30, 2, 2026)) == 0);
    CHECK(Recurring::generateDueTransactions(yearData, Date(31, 4, 2026)) == 0);
    CHECK(Recurring::generateDueTransactions(yearData, Date(0, 1, 2026)) == 0);
    CHECK(yearData.incomes.size() == prevIncomes);
    CHECK(yearData.expenses.size() == prevExpenses);
    CHECK(yearData.nextIncomeNo == prevNextInc);
    CHECK(yearData.nextExpenseNo == prevNextExp);
}

static void testCrossAnnualStatistics() {
    DynamicArray<int> yrs;
    CHECK(!AnnualStatistics::containsYear(yrs, 0));
    CHECK(!AnnualStatistics::containsYear(yrs, 2026));
    CHECK(!AnnualStatistics::containsYear(yrs, -1));

    yrs.add(2026);
    CHECK(AnnualStatistics::containsYear(yrs, 2026));
    CHECK(!AnnualStatistics::containsYear(yrs, 2025));

    yrs.add(0);
    yrs.add(-2020);
    yrs.add(2026);
    yrs.add(2024);
    CHECK(AnnualStatistics::containsYear(yrs, 0));
    CHECK(AnnualStatistics::containsYear(yrs, -2020));
    CHECK(AnnualStatistics::containsYear(yrs, 2026));
    CHECK(AnnualStatistics::containsYear(yrs, 2024));
    CHECK(!AnnualStatistics::containsYear(yrs, 2025));

    FinanceData data;
    data.sources.add(IncomeSource("SRC001", "Salary"));
    data.sources.add(IncomeSource("SRC002", "Bonus"));
    data.sources.add(IncomeSource("SRC003", "Unused"));
    data.categories.add(ExpenseCategory("CAT001", "Food"));
    data.categories.add(ExpenseCategory("CAT002", "Rent"));
    data.categories.add(ExpenseCategory("CAT003", "Unused"));
    data.wallets.add(Wallet("WAL001", "Bank"));

    data.incomes.add(Income("INC0001", Date(31, 12, 2025), "SRC001", 3000000000LL, "WAL001", ""));
    data.incomes.add(Income("INC0002", Date(1, 1, 2026), "SRC001", 3500000000LL, "WAL001", ""));
    data.incomes.add(Income("INC0003", Date(15, 6, 2026), "SRC001", 2500000000LL, "WAL001", ""));
    data.incomes.add(Income("INC0004", Date(20, 7, 2026), "SRC002", 400000LL, "WAL001", ""));
    data.incomes.add(Income("INC0005", Date(5, 5, 2023), "SRC002", 1200000LL, "WAL001", ""));
    data.incomes.add(Income("INC0006", Date(10, 8, 2026), "SRC999", 999999LL, "WAL001", ""));

    data.expenses.add(Expense("EXP0001", Date(31, 12, 2025), "CAT001", 100000LL, "WAL001", ""));
    data.expenses.add(Expense("EXP0002", Date(1, 1, 2026), "CAT002", 3100000000LL, "WAL001", ""));
    data.expenses.add(Expense("EXP0003", Date(10, 2, 2026), "CAT002", 2900000000LL, "WAL001", ""));
    data.expenses.add(Expense("EXP0004", Date(12, 3, 2023), "CAT001", 500000LL, "WAL001", ""));
    data.expenses.add(Expense("EXP0005", Date(18, 9, 2026), "CAT999", 777777LL, "WAL001", ""));

    DynamicArray<int> emptyYears;
    Statistics::PeriodSummary emptySum = AnnualStatistics::summarizeYears(data, emptyYears);
    CHECK(emptySum.totalIncome == 0LL);
    CHECK(emptySum.totalExpense == 0LL);
    CHECK(emptySum.net() == 0LL);

    DynamicArray<int> noMatchYears;
    noMatchYears.add(1999);
    noMatchYears.add(2035);
    Statistics::PeriodSummary noMatchSum = AnnualStatistics::summarizeYears(data, noMatchYears);
    CHECK(noMatchSum.totalIncome == 0LL);
    CHECK(noMatchSum.totalExpense == 0LL);
    CHECK(noMatchSum.net() == 0LL);

    DynamicArray<int> y2025;
    y2025.add(2025);
    Statistics::PeriodSummary s2025 = AnnualStatistics::summarizeYears(data, y2025);
    CHECK(s2025.totalIncome == 3000000000LL);
    CHECK(s2025.totalExpense == 100000LL);
    CHECK(s2025.net() == 2999900000LL);

    DynamicArray<int> y2026;
    y2026.add(2026);
    Statistics::PeriodSummary s2026 = AnnualStatistics::summarizeYears(data, y2026);
    CHECK(s2026.totalIncome == 6001399999LL);
    CHECK(s2026.totalExpense == 6000777777LL);
    CHECK(s2026.net() == 622222LL);

    DynamicArray<int> dup2026;
    dup2026.add(2026);
    dup2026.add(2026);
    dup2026.add(2026);
    Statistics::PeriodSummary sDup = AnnualStatistics::summarizeYears(data, dup2026);
    CHECK(sDup.totalIncome == s2026.totalIncome);
    CHECK(sDup.totalExpense == s2026.totalExpense);
    CHECK(sDup.net() == s2026.net());

    DynamicArray<int> nonContig;
    nonContig.add(2023);
    nonContig.add(2026);
    Statistics::PeriodSummary sNonContig = AnnualStatistics::summarizeYears(data, nonContig);
    CHECK(sNonContig.totalIncome == 6002599999LL);
    CHECK(sNonContig.totalExpense == 6001277777LL);
    CHECK(sNonContig.net() == 1322222LL);

    DynamicArray<long long> inc2026 = AnnualStatistics::incomeBySource(data, dup2026);
    CHECK(inc2026.size() == 3);
    CHECK(inc2026[0] == 6000000000LL);
    CHECK(inc2026[1] == 400000LL);
    CHECK(inc2026[2] == 0LL);

    DynamicArray<long long> exp2026 = AnnualStatistics::expenseByCategory(data, dup2026);
    CHECK(exp2026.size() == 3);
    CHECK(exp2026[0] == 0LL);
    CHECK(exp2026[1] == 6000000000LL);
    CHECK(exp2026[2] == 0LL);

    DynamicArray<long long> incNonContig = AnnualStatistics::incomeBySource(data, nonContig);
    CHECK(incNonContig.size() == 3);
    CHECK(incNonContig[0] == 6000000000LL);
    CHECK(incNonContig[1] == 1600000LL);
    CHECK(incNonContig[2] == 0LL);

    DynamicArray<long long> expNonContig = AnnualStatistics::expenseByCategory(data, nonContig);
    CHECK(expNonContig.size() == 3);
    CHECK(expNonContig[0] == 500000LL);
    CHECK(expNonContig[1] == 6000000000LL);
    CHECK(expNonContig[2] == 0LL);

    FinanceData noMaster;
    noMaster.incomes.add(Income("INC0001", Date(1, 1, 2026), "SRC001", 1000LL, "WAL001", ""));
    noMaster.expenses.add(Expense("EXP0001", Date(1, 1, 2026), "CAT001", 500LL, "WAL001", ""));
    CHECK(AnnualStatistics::incomeBySource(noMaster, y2026).isEmpty());
    CHECK(AnnualStatistics::expenseByCategory(noMaster, y2026).isEmpty());
}

static void testCrossKhoaWithOtherModules() {
    using MasterData::Kind;
    FinanceData data;
    std::string msg;

    CHECK(MasterData::addItem(data, Kind::WALLET, "Cash", msg));
    CHECK(MasterData::addItem(data, Kind::WALLET, "Bank", msg));
    CHECK(MasterData::addItem(data, Kind::SOURCE, "Salary", msg));
    CHECK(MasterData::addItem(data, Kind::SOURCE, "Dividend", msg));
    CHECK(MasterData::addItem(data, Kind::CATEGORY, "Rent", msg));
    CHECK(MasterData::addItem(data, Kind::CATEGORY, "Utilities", msg));

    CHECK(Recurring::addRecurring(data, RecurringTransaction("", TransactionType::INCOME, "SRC001", 3000000LL, "WAL002", "Salary", Date(25, 11, 2025), false, Date())) == "REC001");
    CHECK(Recurring::addRecurring(data, RecurringTransaction("", TransactionType::EXPENSE, "CAT001", 1000000LL, "WAL002", "Rent", Date(5, 12, 2025), true, Date(5, 1, 2026))) == "REC002");
    CHECK(Recurring::addRecurring(data, RecurringTransaction("", TransactionType::EXPENSE, "CAT002", 200000LL, "WAL001", "Electric", Date(10, 11, 2025), false, Date())) == "REC003");

    CHECK(!MasterData::removeItem(data, Kind::WALLET, "WAL001", msg));
    CHECK(!MasterData::removeItem(data, Kind::SOURCE, "SRC001", msg));
    CHECK(!MasterData::removeItem(data, Kind::CATEGORY, "CAT001", msg));

    CHECK(Recurring::generateDueTransactions(data, Date(15, 11, 2025)) == 2);
    CHECK(Recurring::generateDueTransactions(data, Date(20, 12, 2025)) == 3);
    CHECK(Recurring::generateDueTransactions(data, Date(15, 1, 2026)) == 3);
    CHECK(data.incomes.size() == 3);
    CHECK(data.expenses.size() == 5);

    const std::string path = "test_cross_khoa.dat";
    std::remove(path.c_str());
    CHECK(Storage::save(data, path));

    FinanceData reloaded;
    CHECK(Storage::load(reloaded, path));
    std::remove(path.c_str());

    CHECK(Recurring::generateDueTransactions(reloaded, Date(28, 1, 2026)) == 0);
    CHECK(reloaded.incomes.size() == 3);
    CHECK(reloaded.expenses.size() == 5);

    CHECK(Balance::walletBalance(reloaded, "WAL001") == -600000LL);
    CHECK(Balance::walletBalance(reloaded, "WAL002") == 7000000LL);
    CHECK(Balance::totalBalance(reloaded) == 6400000LL);

    DynamicArray<int> y2025;
    y2025.add(2025);
    DynamicArray<int> y2026;
    y2026.add(2026);
    DynamicArray<int> bothYears;
    bothYears.add(2025);
    bothYears.add(2026);

    Statistics::PeriodSummary sum2025 = AnnualStatistics::summarizeYears(reloaded, y2025);
    CHECK(sum2025.totalIncome == 6000000LL);
    CHECK(sum2025.totalExpense == 1400000LL);
    CHECK(sum2025.net() == 4600000LL);

    Statistics::PeriodSummary sum2026 = AnnualStatistics::summarizeYears(reloaded, y2026);
    CHECK(sum2026.totalIncome == 3000000LL);
    CHECK(sum2026.totalExpense == 1200000LL);
    CHECK(sum2026.net() == 1800000LL);

    DynamicArray<long long> incBySrc = AnnualStatistics::incomeBySource(reloaded, bothYears);
    CHECK(incBySrc.size() == 2);
    CHECK(incBySrc[0] == 9000000LL);
    CHECK(incBySrc[1] == 0LL);

    DynamicArray<long long> expByCat = AnnualStatistics::expenseByCategory(reloaded, bothYears);
    CHECK(expByCat.size() == 2);
    CHECK(expByCat[0] == 2000000LL);
    CHECK(expByCat[1] == 600000LL);

    Income updatedInc(reloaded.incomes[2].getId(), Date(25, 1, 2026), "SRC002", 4500000LL, "WAL001", "Adjusted", reloaded.incomes[2].getRecurringId());
    CHECK(Transactions::updateIncome(reloaded, 2, updatedInc));

    CHECK(Balance::walletBalance(reloaded, "WAL001") == 3900000LL);
    CHECK(Balance::walletBalance(reloaded, "WAL002") == 4000000LL);
    CHECK(Balance::totalBalance(reloaded) == 7900000LL);

    sum2026 = AnnualStatistics::summarizeYears(reloaded, y2026);
    CHECK(sum2026.totalIncome == 4500000LL);
    CHECK(sum2026.totalExpense == 1200000LL);

    incBySrc = AnnualStatistics::incomeBySource(reloaded, bothYears);
    CHECK(incBySrc[0] == 6000000LL);
    CHECK(incBySrc[1] == 4500000LL);

    CHECK(Recurring::generateDueTransactions(reloaded, Date(31, 1, 2026)) == 0);

    Transactions::TransactionRef delRef;
    delRef.isIncome = false;
    delRef.index = 1;
    CHECK(reloaded.expenses[1].getCategoryId() == "CAT001");
    CHECK(reloaded.expenses[1].getDate().getYear() == 2025);
    CHECK(Transactions::deleteTransaction(reloaded, delRef));

    CHECK(Balance::walletBalance(reloaded, "WAL002") == 5000000LL);
    CHECK(Balance::totalBalance(reloaded) == 8900000LL);

    sum2025 = AnnualStatistics::summarizeYears(reloaded, y2025);
    CHECK(sum2025.totalIncome == 6000000LL);
    CHECK(sum2025.totalExpense == 400000LL);

    expByCat = AnnualStatistics::expenseByCategory(reloaded, bothYears);
    CHECK(expByCat[0] == 1000000LL);
    CHECK(expByCat[1] == 600000LL);
}

int main() {
    testDynamicArray();
    testDate();
    testHelpers();
    testBalance();
    testMasterDataLookup();
    testMasterDataAddItem();
    testMasterDataDuplicateNames();
    testMasterDataTabTrimming();
    testMasterDataRemoveInUse();
    testMasterDataSourceIdNotReused();
    testMasterDataRemoveMiddleItem();
    testMasterDataInUseAfterTransactionDeletion();
    testMasterDataInUseAfterIncomeUpdate();
    testMasterDataInUseAfterExpenseUpdate();
    testBalanceAfterTransactionChanges();
    testBalanceNegativeWallet();

    testSortedTransactions();
    testDeleteTransaction();
    testUpdateTransactions();
    testRecurringCrud();
    testContainsYear();
    testGenerateDueTransactions();
    testAnnualStatistics();
    testRecurringEdgeCases();
    testAnnualStatisticsEdgeCases();
    testStorageEdgeCases();
    testCrossAddTransactions();
    testCrossSortedOrder();
    testCrossUpdateTransactions();
    testCrossDeleteTransactions();
    testCrossTransactionsWithOtherModules();
    testAuth();
    testCrossRecurringCrud();
    testCrossGenerateDueTransactions();
    testCrossAnnualStatistics();
    testCrossKhoaWithOtherModules();
    if (failures == 0) {
        std::cout << "All tests passed.\n";
        return 0;
    }

    std::cout << failures << " test(s) failed.\n";
    return 1;
}
