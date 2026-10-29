#include "gui/Pages.h"

#include "imgui.h"
#include "implot.h"
#include "IconsFontAwesome6.h"
#include "gui/Theme.h"
#include "gui/Widgets.h"
#include "Balance.h"
#include "Format.h"
#include "MasterData.h"
#include "Statistics.h"

using Theme::px;

namespace {
    void rightText(const std::string& text, const ImVec4& color) {
        ImGui::SameLine();
        float w = ImGui::CalcTextSize(text.c_str()).x;
        float x = ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - w;
        if (x > ImGui::GetCursorPosX()) ImGui::SetCursorPosX(x);
        ImGui::TextColored(color, "%s", text.c_str());
    }

    // Soft colored circle with a short text (a letter or an icon) in the middle.
    void avatar(const std::string& name, const ImVec4& color) {
        float r = px(16);
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImVec2 center(pos.x + r, pos.y + r);
        ImDrawList* draw = ImGui::GetWindowDrawList();
        draw->AddCircleFilled(center, r, Theme::toU32(color, 0.16f));
        std::string letter = name.empty() ? "?" : name;
        ImVec2 size = ImGui::CalcTextSize(letter.c_str());
        draw->AddText(ImVec2(center.x - size.x * 0.5f, center.y - size.y * 0.5f), Theme::toU32(color), letter.c_str());
        ImGui::Dummy(ImVec2(r * 2, r * 2));
    }

    void walletCard(Gui::AppState& app, const ImVec2& size) {
        const Theme::Palette& p = Theme::colors();
        const FinanceData& data = app.data;
        Widgets::beginCard("##wallets", size);
        Widgets::cardTitle(ICON_FA_BUILDING_COLUMNS, "Wallet balances");
        ImGui::Spacing();

        int n = data.wallets.size();
        if (n == 0) {
            Widgets::emptyState(ICON_FA_WALLET, "No wallets yet", "Add one in Master Data.");
            if (Widgets::button(ICON_FA_PLUS "  Add wallet", p.blue)) {
                app.page = Gui::Page::MASTER_DATA;
            }
            Widgets::endCard();
            return;
        }

        const ImVec4 colors[] = { p.blue, p.green, p.orange, p.purple, p.teal, p.red };
        float listWidth = ImGui::GetContentRegionAvail().x * 0.42f;
        ImGui::BeginChild("##walletList", ImVec2(listWidth, 0), ImGuiChildFlags_None, ImGuiWindowFlags_NoBackground);
        DynamicArray<double> balances;
        for (int i = 0; i < n; i++) {
            const Wallet& w = data.wallets[i];
            long long balance = Balance::walletBalance(data, w.getId());
            balances.add((double)balance);
            ImGui::PushID(i);
            float rowY = ImGui::GetCursorPosY();
            avatar(w.getName().substr(0, 1), colors[i % 6]);
            ImGui::SameLine();
            ImGui::SetCursorPosY(rowY + px(6));
            ImGui::TextUnformatted(w.getName().c_str());
            ImGui::SameLine();
            ImGui::SetCursorPosY(rowY + px(6));
            std::string money = Format::money(balance);
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(money.c_str()).x);
            ImGui::TextColored(balance < 0 ? p.red : p.text, "%s", money.c_str());
            ImGui::SetCursorPosY(rowY + px(32) + ImGui::GetStyle().ItemSpacing.y);
            ImGui::PopID();
        }
        ImGui::Separator();
        ImGui::PushFont(Theme::boldFont(), 0.0f);
        ImGui::TextUnformatted("Total");
        long long total = Balance::totalBalance(data);
        rightText(Format::money(total) + " ₫", total < 0 ? p.red : p.green);
        ImGui::PopFont();
        ImGui::EndChild();

        ImGui::SameLine(0, px(20));
        DynamicArray<std::string> names;
        for (int i = 0; i < n; i++) names.add(data.wallets[i].getName());
        const char** labels = new const char*[n];
        double* positions = new double[n];
        ImU32* barColors = new ImU32[n];
        for (int i = 0; i < n; i++) {
            labels[i] = names[i].c_str();
            positions[i] = i;
            barColors[i] = Theme::toU32(balances[i] < 0 ? p.red : colors[i % 6]);
        }
        if (ImPlot::BeginPlot("##walletChart", ImVec2(-1, -1),
                              ImPlotFlags_NoLegend | ImPlotFlags_NoMenus | ImPlotFlags_NoBoxSelect |
                              ImPlotFlags_NoMouseText | ImPlotFlags_NoInputs)) {
            Widgets::setupBarAxes(positions, n, labels, Widgets::maxAbs(balances.rawData(), n));
            ImPlotSpec spec;
            spec.FillColors = barColors;
            spec.LineColors = barColors;
            ImPlot::PlotBars("Balance", balances.rawData(), n, 0.55, 0, spec);
            ImPlot::EndPlot();
        }
        delete[] labels;
        delete[] positions;
        delete[] barColors;
        Widgets::endCard();
    }

    void recentCard(Gui::AppState& app, const ImVec2& size) {
        const Theme::Palette& p = Theme::colors();
        const FinanceData& data = app.data;
        Widgets::beginCard("##recent", size);
        Widgets::cardTitle(ICON_FA_CLOCK_ROTATE_LEFT, "Recent transactions");
        ImGui::Spacing();

        DynamicArray<Transactions::TransactionRef> refs = Transactions::getSortedTransactions(data);
        if (refs.isEmpty()) {
            Widgets::emptyState(ICON_FA_RECEIPT, "No transactions yet", "Use \"Add income\" or \"Add expense\".");
            if (!data.incomes.isEmpty() || !data.expenses.isEmpty()) {
                Widgets::todoNote("Transactions::getSortedTransactions()");
            }
        }
        int shown = refs.size() < 4 ? refs.size() : 4;
        for (int i = 0; i < shown; i++) {
            Pages::TransactionView t = Pages::viewOf(data, refs[i]);
            ImVec4 color = t.isIncome ? p.green : p.red;
            ImVec2 start = ImGui::GetCursorScreenPos();
            float lineHeight = ImGui::GetTextLineHeightWithSpacing();
            ImGui::GetWindowDrawList()->AddRectFilled(start, ImVec2(start.x + px(3), start.y + lineHeight * 2 - px(4)),
                                                      Theme::toU32(color), px(2));
            ImGui::Indent(px(14));
            ImGui::PushFont(Theme::boldFont(), 0.0f);
            ImGui::TextColored(color, "%s", Format::signedMoney(t.isIncome ? t.amount : -t.amount).c_str());
            ImGui::PopFont();
            rightText(t.date.toString(), p.textMuted);
            Widgets::mutedText(MasterData::getWalletName(data, t.walletId) + "   " ICON_FA_ANGLE_RIGHT "   "
                               + Pages::refNameOf(data, t));
            ImGui::Unindent(px(14));
            if (i < shown - 1) ImGui::Separator();
        }
        if (refs.size() > shown) {
            ImGui::Spacing();
            if (Widgets::ghostButton("View all transactions  " ICON_FA_ARROW_RIGHT, ImVec2(-1, 0))) {
                app.page = Gui::Page::TRANSACTIONS;
            }
        }
        Widgets::endCard();
    }

    void monthlyCard(const Gui::AppState& app, const ImVec2& size) {
        const Theme::Palette& p = Theme::colors();
        Widgets::beginCard("##monthly", size);
        Widgets::cardTitle(ICON_FA_CHART_COLUMN, "Income vs expense - last 6 months");
        ImGui::Spacing();

        Date today = Date::today();
        double income[6], expense[6], positions[6];
        std::string names[6];
        const char* labels[6];
        for (int i = 0; i < 6; i++) {
            int month = today.getMonth() - 5 + i;
            int year = today.getYear();
            while (month < 1) { month += 12; year--; }
            Date from(1, month, year);
            Date to(Date::daysInMonth(month, year), month, year);
            Statistics::PeriodSummary s = Statistics::summarize(app.data, from, to);
            income[i] = (double)s.totalIncome;
            expense[i] = (double)s.totalExpense;
            positions[i] = i;
            names[i] = Format::monthName(month) + " " + std::to_string(year % 100);
            labels[i] = names[i].c_str();
        }

        if (ImPlot::BeginPlot("##monthlyChart", ImVec2(-1, -1),
                              ImPlotFlags_NoMenus | ImPlotFlags_NoBoxSelect | ImPlotFlags_NoMouseText | ImPlotFlags_NoInputs)) {
            ImPlot::SetupLegend(ImPlotLocation_NorthEast, ImPlotLegendFlags_Horizontal);
            double top = Widgets::maxAbs(income, 6);
            if (Widgets::maxAbs(expense, 6) > top) top = Widgets::maxAbs(expense, 6);
            Widgets::setupBarAxes(positions, 6, labels, top);
            ImPlotSpec incomeSpec;
            incomeSpec.FillColor = p.green;
            incomeSpec.LineColor = p.green;
            ImPlotSpec expenseSpec;
            expenseSpec.FillColor = p.red;
            expenseSpec.LineColor = p.red;
            ImPlot::PlotBars("Income", income, 6, 0.3, -0.16, incomeSpec);
            ImPlot::PlotBars("Expense", expense, 6, 0.3, 0.16, expenseSpec);
            ImPlot::EndPlot();
        }
        Widgets::endCard();
    }

    void recurringCard(Gui::AppState& app, const ImVec2& size) {
        const Theme::Palette& p = Theme::colors();
        const FinanceData& data = app.data;
        Widgets::beginCard("##recurringCard", size);
        Widgets::cardTitle(ICON_FA_ARROWS_ROTATE, "Recurring");
        ImGui::Spacing();
        if (data.recurrings.isEmpty()) {
            Widgets::emptyState(ICON_FA_ARROWS_ROTATE, "No recurring items", "Salary, rent, subscriptions...");
        }
        Date today = Date::today();
        int shown = data.recurrings.size() < 5 ? data.recurrings.size() : 5;
        for (int i = 0; i < shown; i++) {
            const RecurringTransaction& r = data.recurrings[i];
            bool income = r.getType() == TransactionType::INCOME;
            std::string name = income ? MasterData::getSourceName(data, r.getRefId())
                                      : MasterData::getCategoryName(data, r.getRefId());
            ImGui::PushID(i);
            avatar(income ? ICON_FA_ARROW_DOWN : ICON_FA_ARROW_UP, income ? p.green : p.red);
            ImGui::SameLine();
            ImGui::BeginGroup();
            ImGui::TextUnformatted(r.getDescription().empty() ? name.c_str() : r.getDescription().c_str());
            Widgets::mutedText(Format::signedMoney(income ? r.getAmount() : -r.getAmount()) + " / month");
            ImGui::EndGroup();

            bool ended = r.getHasEndDate() && r.getEndDate() < today;
            bool scheduled = today < r.getStartDate();
            const char* status = ended ? "Ended" : (scheduled ? "Scheduled" : "Active");
            ImVec4 color = ended ? p.textMuted : (scheduled ? p.blue : p.green);
            ImGui::SameLine();
            float w = ImGui::CalcTextSize(status).x + px(16);
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - w);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + px(6));
            Widgets::badge(status, color);
            ImGui::PopID();
        }
        if (Widgets::ghostButton("Manage recurring  " ICON_FA_ARROW_RIGHT, ImVec2(-1, 0))) {
            app.page = Gui::Page::RECURRING;
        }
        Widgets::endCard();
    }
}

namespace Pages {

void dashboard(Gui::AppState& app) {
    const Theme::Palette& p = Theme::colors();
    const FinanceData& data = app.data;

    const char* addIncome = ICON_FA_PLUS "  Add income";
    const char* addExpense = ICON_FA_MINUS "  Add expense";
    Widgets::pageHeader("Dashboard", "Overview of all your wallets and this month's activity",
                        Widgets::buttonWidth(addIncome) + Widgets::buttonWidth(addExpense) + px(10));
    if (Widgets::button(addIncome, p.green)) openAddTransaction(true);
    ImGui::SameLine();
    if (Widgets::button(addExpense, p.red)) openAddTransaction(false);
    Widgets::headerEnd();
    ImGui::Dummy(ImVec2(0, px(6)));

    Date today = Date::today();
    Date from(1, today.getMonth(), today.getYear());
    Date to(Date::daysInMonth(today.getMonth(), today.getYear()), today.getMonth(), today.getYear());
    Statistics::PeriodSummary month = Statistics::summarize(data, from, to);
    long long total = Balance::totalBalance(data);
    std::string monthText = Format::monthName(today.getMonth()) + " " + std::to_string(today.getYear());

    float gap = px(18);
    float avail = ImGui::GetContentRegionAvail().x;
    ImVec2 cardSize((avail - gap * 3) / 4, px(176));
    std::string walletCaption = std::to_string(data.wallets.size()) + " wallet(s)";
    Widgets::statCard("##c1", ICON_FA_WALLET, total < 0 ? p.red : p.green, "Current Balance",
                      Format::money(total) + " ₫", cardSize, walletCaption.c_str());
    ImGui::SameLine(0, gap);
    Widgets::statCard("##c2", ICON_FA_ARROW_TREND_UP, p.blue, "This Month's Income",
                      Format::money(month.totalIncome) + " ₫", cardSize, monthText.c_str());
    ImGui::SameLine(0, gap);
    Widgets::statCard("##c3", ICON_FA_CART_SHOPPING, p.orange, "This Month's Expense",
                      Format::money(month.totalExpense) + " ₫", cardSize, monthText.c_str());
    ImGui::SameLine(0, gap);
    Widgets::statCard("##c4", ICON_FA_PIGGY_BANK, month.net() < 0 ? p.red : p.purple, "This Month's Savings",
                      Format::signedMoney(month.net()) + " ₫", cardSize, "Income - Expense");
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + gap - ImGui::GetStyle().ItemSpacing.y);

    float leftWidth = (avail - gap) * 0.62f;
    float rightWidth = avail - gap - leftWidth;
    walletCard(app, ImVec2(leftWidth, px(330)));
    ImGui::SameLine(0, gap);
    recentCard(app, ImVec2(rightWidth, px(330)));
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + gap - ImGui::GetStyle().ItemSpacing.y);

    monthlyCard(app, ImVec2(leftWidth, px(320)));
    ImGui::SameLine(0, gap);
    recurringCard(app, ImVec2(rightWidth, px(320)));
}

}
