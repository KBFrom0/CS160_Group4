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

    // renameItem: unknown id -> rejected with a message.
    message = "";
    CHECK(!MasterData::renameItem(data, Kind::WALLET, "WAL999", "Anything", message));
    CHECK(message != "");
    CHECK(!MasterData::renameItem(data, Kind::WALLET, "CAT001", "Anything", message));

    // The ID never changes when renaming.
    CHECK(data.wallets[0].getId() == "WAL001");
}

int main() {
    testDynamicArray();
    testDate();
    testHelpers();
    testBalance();
    testMasterDataLookup();
    testMasterDataAddItem();
    testMasterDataDuplicateNames();

    testSortedTransactions();
    testDeleteTransaction();
    testUpdateTransactions();
    testRecurringCrud();
    testContainsYear();
    if (failures == 0) {
        std::cout << "All tests passed.\n";
        return 0;
    }

    std::cout << failures << " test(s) failed.\n";
    return 1;
}
