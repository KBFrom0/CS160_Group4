// Unit tests for the core building blocks (DynamicArray, Date, FinanceData::makeId).
// Build & run: see README.md. Prints FAIL lines and returns 1 if anything breaks.
#include "Transactions.h"
#include <iostream>
#include <string>
#include "DynamicArray.h"
#include "Date.h"
#include "FinanceData.h"
#include "Format.h"

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

int main() {
    testDynamicArray();
    testDate();
    testHelpers();

    testSortedTransactions();
    testDeleteTransaction();
    testUpdateTransactions();

    if (failures == 0) {
        std::cout << "All tests passed.\n";
        return 0;
    }

    std::cout << failures << " test(s) failed.\n";
    return 1;
}
