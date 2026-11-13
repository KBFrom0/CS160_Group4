#include "Statistics.h"

#include <iostream>
#include "InputHelper.h"
#include "MasterData.h"
#include "AnnualStatistics.h"

namespace Statistics {

PeriodSummary summarize(const FinanceData& data, const Date& from, const Date& to,
                        const std::string& walletId) {
    PeriodSummary result;

    for (int i = 0; i < data.incomes.size(); i++) {
        const Income& inc = data.incomes.get(i);
        if (!inc.getDate().isBetween(from, to))
            continue;
        if (!walletId.empty() && inc.getWalletId() != walletId)
            continue;
        result.totalIncome += inc.getAmount();
    }

    for (int i = 0; i < data.expenses.size(); i++) {
        const Expense& exp = data.expenses.get(i);
        if (!exp.getDate().isBetween(from, to))
            continue;
        if (!walletId.empty() && exp.getWalletId() != walletId)
            continue;
        result.totalExpense += exp.getAmount();
    }

    return result;
}

static void showDateRangeReport(const FinanceData& data) {
    using namespace InputHelper;

    std::cout << "\n INCOME / EXPENSE IN A DATE RANGE\n";
    printLine();

    Date from = inputDate(" From date (dd/mm/yyyy): ");
    Date to = inputDate(" To date   (dd/mm/yyyy): ");

    if (to < from) {
        std::cout << " Error: 'To' date must not be before 'From' date.\n";
        pause();
        return;
    }

    PeriodSummary s = summarize(data, from, to);

    std::cout << "\n";
    printLine();
    std::cout << " Period: " << from.toString() << " -> " << to.toString() << "\n";
    printLine();
    std::cout << " Total income  : " << formatMoney(s.totalIncome) << "\n";
    std::cout << " Total expense : " << formatMoney(s.totalExpense) << "\n";
    printLine();
    std::cout << " Net (income - expense): " << formatMoney(s.net()) << "\n";
    printLine();
    std::cout << "\n";
    pause();
}

static void showDateRangeByWallet(const FinanceData& data) {
    using namespace InputHelper;

    std::cout << "\n INCOME / EXPENSE IN A DATE RANGE BY WALLET\n";
    printLine();

    Date from = inputDate(" From date (dd/mm/yyyy): ");
    Date to = inputDate(" To date   (dd/mm/yyyy): ");

    if (to < from) {
        std::cout << " Error: 'To' date must not be before 'From' date.\n";
        pause();
        return;
    }

    std::cout << "\n";
    printLine();
    std::cout << " Period: " << from.toString() << " -> " << to.toString() << "\n";
    printLine();

    long long grandIncome = 0;
    long long grandExpense = 0;

    for (int i = 0; i < data.wallets.size(); i++) {
        const Wallet& w = data.wallets.get(i);
        PeriodSummary s = summarize(data, from, to, w.getId());
        grandIncome += s.totalIncome;
        grandExpense += s.totalExpense;

        std::cout << " " << w.getName() << ":\n";
        std::cout << "   Income  : " << formatMoney(s.totalIncome) << "\n";
        std::cout << "   Expense : " << formatMoney(s.totalExpense) << "\n";
        std::cout << "   Net     : " << formatMoney(s.net()) << "\n";
    }

    printLine();
    std::cout << " TOTAL:\n";
    std::cout << "   Income  : " << formatMoney(grandIncome) << "\n";
    std::cout << "   Expense : " << formatMoney(grandExpense) << "\n";
    std::cout << "   Net     : " << formatMoney(grandIncome - grandExpense) << "\n";
    printLine();
    std::cout << "\n";
    pause();
}

void showMenu(const FinanceData& data) {
    using namespace InputHelper;

    while (true) {
        clearScreen();
        std::cout << "\n STATISTICS & REPORTS\n";
        printLine();
        std::cout << " 1. Income / expense in a date range\n";
        std::cout << " 2. Income / expense in a date range by wallet\n";
        std::cout << " 3. Annual overview\n";
        std::cout << " 4. Annual income by source\n";
        std::cout << " 5. Annual expense by category\n";
        std::cout << " 0. Back\n";
        printLine();

        int choice = inputInt(" Your choice: ", 0, 5);
        switch (choice) {
            case 1: showDateRangeReport(data); break;
            case 2: showDateRangeByWallet(data); break;
            case 3: AnnualStatistics::showOverview(data); break;
            case 4: AnnualStatistics::showIncomeBySource(data); break;
            case 5: AnnualStatistics::showExpenseByCategory(data); break;
            case 0: return;
        }
    }
}

}
