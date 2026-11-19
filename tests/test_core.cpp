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
#include "DemoData.h"
#include "MasterData.h"
#include "Recurring.h"
#include "AnnualStatistics.h"
#include "Storage.h"
#include <cstdio>

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

    // Found at the first, middle and last position.
    CHECK(MasterData::findCategoryIndex(data, "CAT001") == 0);
    CHECK(MasterData::findSourceIndex(data, "SRC002") == 1);
    CHECK(MasterData::findSourceIndex(data, "SRC003") == 2);
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

    // Sample data, worked out by hand from DemoData.cpp.
    FinanceData demo;
    DemoData::fill(demo);
    CHECK(Balance::walletBalance(demo, "WAL001") == 730000);
    CHECK(Balance::walletBalance(demo, "WAL002") == 47350000);
    CHECK(Balance::walletBalance(demo, "WAL003") == -3500000);
    CHECK(Balance::totalBalance(demo) == 44580000);
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
}

int main() {
    testDynamicArray();
    testDate();
    testHelpers();
    testBalance();
    testMasterDataLookup();
    testMasterDataAddItem();

    testSortedTransactions();
    testDeleteTransaction();
    testUpdateTransactions();
    testRecurringCrud();
    testContainsYear();
    testStorageEdgeCases();
    if (failures == 0) {
        std::cout << "All tests passed.\n";
        return 0;
    }

    std::cout << failures << " test(s) failed.\n";
    return 1;
}
