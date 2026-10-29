#include "gui/Pages.h"

#include <cstdio>
#include "imgui.h"
#include "implot.h"
#include "IconsFontAwesome6.h"
#include "gui/Theme.h"
#include "gui/Widgets.h"
#include "AnnualStatistics.h"
#include "Format.h"
#include "MasterData.h"
#include "Statistics.h"

using Theme::px;

namespace {
    int tab = 0;
    Date rangeFrom(1, Date::today().getMonth(), Date::today().getYear());
    Date rangeTo = Date::today();
    DynamicArray<int> selectedYears;

    ImVec4 seriesColor(int i) {
        const Theme::Palette& p = Theme::colors();
        const ImVec4 colors[] = { p.blue, p.orange, p.green, p.purple, p.teal, p.red };
        return colors[i % 6];
    }

    void gapY() {
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + px(18) - ImGui::GetStyle().ItemSpacing.y);
    }

    std::string vnd(long long amount) { return Format::money(amount) + " ₫"; }

    // ---------- Date range controls ----------
    bool rangeControls() {
        const Theme::Palette& p = Theme::colors();
        Widgets::beginCard("##rangeControls");
        Date today = Date::today();
        ImGui::AlignTextToFramePadding();
        Widgets::mutedText("From");
        ImGui::SameLine();
        Widgets::dateInput("from", rangeFrom, px(170));
        ImGui::SameLine(0, px(14));
        ImGui::AlignTextToFramePadding();
        Widgets::mutedText("To");
        ImGui::SameLine();
        Widgets::dateInput("to", rangeTo, px(170));
        ImGui::SameLine(0, px(24));

        int m = today.getMonth(), y = today.getYear();
        int lastMonth = m == 1 ? 12 : m - 1, lastMonthYear = m == 1 ? y - 1 : y;
        if (Widgets::ghostButton("This month")) {
            rangeFrom = Date(1, m, y);
            rangeTo = Date(Date::daysInMonth(m, y), m, y);
        }
        ImGui::SameLine(0, px(6));
        if (Widgets::ghostButton("Last month")) {
            rangeFrom = Date(1, lastMonth, lastMonthYear);
            rangeTo = Date(Date::daysInMonth(lastMonth, lastMonthYear), lastMonth, lastMonthYear);
        }
        ImGui::SameLine(0, px(6));
        if (Widgets::ghostButton("This year")) {
            rangeFrom = Date(1, 1, y);
            rangeTo = Date(31, 12, y);
        }
        ImGui::SameLine(0, px(6));
        if (Widgets::ghostButton("Last year")) {
            rangeFrom = Date(1, 1, y - 1);
            rangeTo = Date(31, 12, y - 1);
        }
        bool valid = rangeFrom <= rangeTo;
        if (!valid) Widgets::formError("\"From\" must be on or before \"To\".");
        Widgets::endCard();
        (void)p;
        return valid;
    }

    void summaryCards(const Statistics::PeriodSummary& s, const char* caption) {
        const Theme::Palette& p = Theme::colors();
        float gap = px(18);
        ImVec2 size((ImGui::GetContentRegionAvail().x - gap * 2) / 3, px(176));
        Widgets::statCard("##in", ICON_FA_ARROW_TREND_UP, p.green, "Total Income", vnd(s.totalIncome), size, caption);
        ImGui::SameLine(0, gap);
        Widgets::statCard("##out", ICON_FA_ARROW_TREND_DOWN, p.red, "Total Expense", vnd(s.totalExpense), size, caption);
        ImGui::SameLine(0, gap);
        Widgets::statCard("##net", ICON_FA_SCALE_BALANCED, s.net() < 0 ? p.red : p.purple, "Net Balance",
                          Format::signedMoney(s.net()) + " ₫", size, "Income - Expense");
    }

    // Donut chart with the total in the middle and a legend on the right.
    // labels/values must have `count` items.
    void pieChart(const char* id, const char* const labels[], const double* values, int count, float height) {
        const Theme::Palette& p = Theme::colors();
        double total = 0;
        for (int i = 0; i < count; i++) total += values[i];
        if (total <= 0) {
            Widgets::emptyState(ICON_FA_CHART_PIE, "Nothing to show", "No amounts in this period.");
            return;
        }
        double* percents = new double[count];
        for (int i = 0; i < count; i++) percents[i] = values[i] * 100.0 / total;

        float legendWidth = px(220);
        float gap = px(32);
        float avail = ImGui::GetContentRegionAvail().x;
        float size = height > 0 ? height : ImGui::GetContentRegionAvail().y;
        if (size > avail - legendWidth - gap) size = avail - legendWidth - gap;
        if (size < px(120)) size = px(120);
        float startX = ImGui::GetCursorPosX() + (avail - size - gap - legendWidth) * 0.5f;
        float startY = ImGui::GetCursorPosY();
        ImGui::SetCursorPosX(startX);

        if (ImPlot::BeginPlot(id, ImVec2(size, size), ImPlotFlags_NoLegend | ImPlotFlags_NoMouseText |
                                                     ImPlotFlags_NoMenus | ImPlotFlags_NoInputs | ImPlotFlags_NoFrame)) {
            ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoDecorations, ImPlotAxisFlags_NoDecorations);
            ImPlot::SetupAxesLimits(0, 1, 0, 1, ImPlotCond_Always);
            ImPlotSpec spec;
            spec.LineColor = p.card;
            spec.LineWeight = 2.0f;
            ImPlot::PlotPieChart(labels, percents, count, 0.5, 0.5, 0.48, "", 90, spec);   // "" = no slice labels

            // Hole in the middle + total
            ImDrawList* draw = ImPlot::GetPlotDrawList();
            ImVec2 center = ImPlot::PlotToPixels(0.5, 0.5);
            float radius = ImPlot::PlotToPixels(0.5 + 0.48 * 0.58, 0.5).x - center.x;
            ImPlot::PushPlotClipRect();
            draw->AddCircleFilled(center, radius, Theme::toU32(p.card), 64);
            std::string caption = "Total";
            std::string amount = Widgets::compactMoney(total) + " ₫";
            ImFont* bold = Theme::boldFont();
            float big = Theme::FONT_TITLE * Theme::scale();
            float small = Theme::FONT_SMALL * Theme::scale();
            ImVec2 amountSize = bold->CalcTextSizeA(big, 1000.0f, 0.0f, amount.c_str());
            ImVec2 captionSize = Theme::regularFont()->CalcTextSizeA(small, 1000.0f, 0.0f, caption.c_str());
            draw->AddText(Theme::regularFont(), small,
                          ImVec2(center.x - captionSize.x * 0.5f, center.y - amountSize.y * 0.5f - captionSize.y),
                          Theme::toU32(p.textMuted), caption.c_str());
            draw->AddText(bold, big, ImVec2(center.x - amountSize.x * 0.5f, center.y - amountSize.y * 0.5f + px(2)),
                          Theme::toU32(p.text), amount.c_str());
            ImPlot::PopPlotClipRect();
            ImPlot::EndPlot();
        }

        // Legend: dot, name, share
        float rowHeight = ImGui::GetFrameHeightWithSpacing();
        ImGui::SetCursorPos(ImVec2(startX + size + gap, startY + (size - rowHeight * count) * 0.5f));
        ImGui::BeginGroup();
        for (int i = 0; i < count; i++) {
            ImGui::AlignTextToFramePadding();
            Widgets::colorDot(seriesColor(i));
            ImGui::SameLine();
            ImGui::TextUnformatted(labels[i]);
            ImGui::SameLine();
            char share[16];
            std::snprintf(share, sizeof(share), "%.0f%%", percents[i]);
            ImGui::SetCursorPosX(startX + size + gap + legendWidth - ImGui::CalcTextSize(share).x);
            ImGui::PushFont(Theme::boldFont(), 0.0f);
            ImGui::TextUnformatted(share);
            ImGui::PopFont();
        }
        ImGui::EndGroup();
        ImGui::SetCursorPos(ImVec2(startX, startY + size));
        ImGui::Dummy(ImVec2(0, 0));   // tells ImGui the block ends here
        delete[] percents;
    }

    // ---------- Tab 1: date range ----------
    void dateRangeTab(const FinanceData& data) {
        bool valid = rangeControls();
        gapY();
        if (!valid) return;
        Statistics::PeriodSummary s = Statistics::summarize(data, rangeFrom, rangeTo);
        std::string caption = rangeFrom.toString() + " - " + rangeTo.toString();
        summaryCards(s, caption.c_str());
        gapY();

        Widgets::beginCard("##rangePie", ImVec2(0, px(320)));
        Widgets::cardTitle(ICON_FA_CHART_PIE, "Income vs expense");
        const char* labels[] = { "Income", "Expense" };
        double values[] = { (double)s.totalIncome, (double)s.totalExpense };
        pieChart("##pie", labels, values, 2, -1);
        Widgets::endCard();
    }

    // ---------- Tab 2: by wallet ----------
    void walletTab(const FinanceData& data) {
        const Theme::Palette& p = Theme::colors();
        bool valid = rangeControls();
        gapY();
        if (!valid) return;
        int n = data.wallets.size();
        if (n == 0) {
            Widgets::beginCard("##noWallets");
            Widgets::emptyState(ICON_FA_WALLET, "No wallets yet", "Add wallets in Master Data.");
            Widgets::endCard();
            return;
        }

        double* income = new double[n];
        double* expense = new double[n];
        double* positions = new double[n];
        const char** labels = new const char*[n];
        DynamicArray<std::string> names;
        for (int i = 0; i < n; i++) names.add(data.wallets[i].getName());

        float gap = px(18);
        float avail = ImGui::GetContentRegionAvail().x;
        float leftWidth = (avail - gap) * 0.48f;
        Widgets::beginCard("##walletTable", ImVec2(leftWidth, px(380)));
        Widgets::cardTitle(ICON_FA_TABLE, "Per wallet");
        if (ImGui::BeginTable("##wallets", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH |
                                              ImGuiTableFlags_PadOuterX | ImGuiTableFlags_ScrollY)) {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("Wallet", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Income", ImGuiTableColumnFlags_WidthFixed, px(110));
            ImGui::TableSetupColumn("Expense", ImGuiTableColumnFlags_WidthFixed, px(110));
            ImGui::TableSetupColumn("Net", ImGuiTableColumnFlags_WidthFixed, px(110));
            ImGui::TableHeadersRow();
            for (int i = 0; i < n; i++) {
                Statistics::PeriodSummary s = Statistics::summarize(data, rangeFrom, rangeTo, data.wallets[i].getId());
                income[i] = (double)s.totalIncome;
                expense[i] = (double)s.totalExpense;
                positions[i] = i;
                labels[i] = names[i].c_str();
                ImGui::TableNextRow(ImGuiTableRowFlags_None, px(34));
                ImGui::TableNextColumn(); ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted(names[i].c_str());
                ImGui::TableNextColumn(); ImGui::AlignTextToFramePadding(); ImGui::TextColored(p.green, "%s", Format::money(s.totalIncome).c_str());
                ImGui::TableNextColumn(); ImGui::AlignTextToFramePadding(); ImGui::TextColored(p.red, "%s", Format::money(s.totalExpense).c_str());
                ImGui::TableNextColumn(); ImGui::AlignTextToFramePadding(); ImGui::TextColored(s.net() < 0 ? p.red : p.text, "%s", Format::signedMoney(s.net()).c_str());
            }
            Statistics::PeriodSummary all = Statistics::summarize(data, rangeFrom, rangeTo);
            ImGui::TableNextRow(ImGuiTableRowFlags_None, px(34));
            ImGui::PushFont(Theme::boldFont(), 0.0f);
            ImGui::TableNextColumn(); ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("Total");
            ImGui::TableNextColumn(); ImGui::AlignTextToFramePadding(); ImGui::TextColored(p.green, "%s", Format::money(all.totalIncome).c_str());
            ImGui::TableNextColumn(); ImGui::AlignTextToFramePadding(); ImGui::TextColored(p.red, "%s", Format::money(all.totalExpense).c_str());
            ImGui::TableNextColumn(); ImGui::AlignTextToFramePadding(); ImGui::TextColored(all.net() < 0 ? p.red : p.text, "%s", Format::signedMoney(all.net()).c_str());
            ImGui::PopFont();
            ImGui::EndTable();
        }
        Widgets::endCard();

        ImGui::SameLine(0, gap);
        Widgets::beginCard("##walletChartCard", ImVec2(0, px(380)));
        Widgets::cardTitle(ICON_FA_CHART_COLUMN, "Income and expense by wallet");
        if (ImPlot::BeginPlot("##walletBars", ImVec2(-1, -1), ImPlotFlags_NoMenus | ImPlotFlags_NoBoxSelect |
                                                              ImPlotFlags_NoMouseText | ImPlotFlags_NoInputs)) {
            ImPlot::SetupLegend(ImPlotLocation_NorthEast, ImPlotLegendFlags_Horizontal);
            double top = Widgets::maxAbs(income, n);
            if (Widgets::maxAbs(expense, n) > top) top = Widgets::maxAbs(expense, n);
            Widgets::setupBarAxes(positions, n, labels, top);
            ImPlotSpec in; in.FillColor = p.green; in.LineColor = p.green;
            ImPlotSpec out; out.FillColor = p.red; out.LineColor = p.red;
            ImPlot::PlotBars("Income", income, n, 0.3, -0.16, in);
            ImPlot::PlotBars("Expense", expense, n, 0.3, 0.16, out);
            ImPlot::EndPlot();
        }
        Widgets::endCard();

        delete[] income;
        delete[] expense;
        delete[] positions;
        delete[] labels;
    }

    // ---------- Year selection ----------
    void yearSelector(const FinanceData& data) {
        const Theme::Palette& p = Theme::colors();
        int current = Date::today().getYear();
        int minYear = current, maxYear = current;
        for (int i = 0; i < data.incomes.size(); i++) {
            int y = data.incomes[i].getDate().getYear();
            if (y < minYear) minYear = y;
            if (y > maxYear) maxYear = y;
        }
        for (int i = 0; i < data.expenses.size(); i++) {
            int y = data.expenses[i].getDate().getYear();
            if (y < minYear) minYear = y;
            if (y > maxYear) maxYear = y;
        }
        if (selectedYears.isEmpty()) selectedYears.add(current);

        Widgets::beginCard("##years");
        ImGui::AlignTextToFramePadding();
        Widgets::mutedText(ICON_FA_CALENDAR "  Years");
        for (int y = minYear; y <= maxYear; y++) {
            ImGui::SameLine(0, px(8));
            int index = -1;
            for (int i = 0; i < selectedYears.size(); i++) if (selectedYears[i] == y) index = i;
            std::string label = (index >= 0 ? ICON_FA_CHECK "  " : "") + std::to_string(y);
            ImGui::PushID(y);
            bool clicked = index >= 0 ? Widgets::button(label.c_str(), p.blue) : Widgets::ghostButton(label.c_str());
            ImGui::PopID();
            if (clicked) {
                if (index >= 0 && selectedYears.size() > 1) selectedYears.removeAt(index);
                else if (index < 0) selectedYears.add(y);
            }
        }
        ImGui::SameLine(0, px(20));
        if (Widgets::ghostButton("Select all")) {
            selectedYears.clear();
            for (int y = minYear; y <= maxYear; y++) selectedYears.add(y);
        }
        Widgets::endCard();
    }

    std::string yearsCaption() {
        std::string text;
        for (int i = 0; i < selectedYears.size(); i++) {
            if (i > 0) text += ", ";
            text += std::to_string(selectedYears[i]);
        }
        return text;
    }

    // ---------- Tab 3: annual overview ----------
    void annualOverviewTab(const FinanceData& data) {
        const Theme::Palette& p = Theme::colors();
        yearSelector(data);
        gapY();
        Statistics::PeriodSummary all = AnnualStatistics::summarizeYears(data, selectedYears);
        std::string caption = yearsCaption();
        summaryCards(all, caption.c_str());
        gapY();

        // Years in ascending order for the table and chart
        int n = selectedYears.size();
        int* years = new int[n];
        for (int i = 0; i < n; i++) years[i] = selectedYears[i];
        for (int i = 0; i < n; i++)
            for (int j = i + 1; j < n; j++)
                if (years[j] < years[i]) { int t = years[i]; years[i] = years[j]; years[j] = t; }

        double* income = new double[n];
        double* expense = new double[n];
        double* positions = new double[n];
        DynamicArray<std::string> names;
        const char** labels = new const char*[n];
        for (int i = 0; i < n; i++) names.add(std::to_string(years[i]));

        float gap = px(18);
        float leftWidth = (ImGui::GetContentRegionAvail().x - gap) * 0.48f;
        Widgets::beginCard("##yearTable", ImVec2(leftWidth, px(340)));
        Widgets::cardTitle(ICON_FA_TABLE, "Per year");
        if (ImGui::BeginTable("##years", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH |
                                            ImGuiTableFlags_PadOuterX | ImGuiTableFlags_ScrollY)) {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("Year", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Income", ImGuiTableColumnFlags_WidthFixed, px(120));
            ImGui::TableSetupColumn("Expense", ImGuiTableColumnFlags_WidthFixed, px(120));
            ImGui::TableSetupColumn("Net", ImGuiTableColumnFlags_WidthFixed, px(120));
            ImGui::TableHeadersRow();
            for (int i = 0; i < n; i++) {
                DynamicArray<int> one;
                one.add(years[i]);
                Statistics::PeriodSummary s = AnnualStatistics::summarizeYears(data, one);
                income[i] = (double)s.totalIncome;
                expense[i] = (double)s.totalExpense;
                positions[i] = i;
                labels[i] = names[i].c_str();
                ImGui::TableNextRow(ImGuiTableRowFlags_None, px(34));
                ImGui::TableNextColumn(); ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted(names[i].c_str());
                ImGui::TableNextColumn(); ImGui::AlignTextToFramePadding(); ImGui::TextColored(p.green, "%s", Format::money(s.totalIncome).c_str());
                ImGui::TableNextColumn(); ImGui::AlignTextToFramePadding(); ImGui::TextColored(p.red, "%s", Format::money(s.totalExpense).c_str());
                ImGui::TableNextColumn(); ImGui::AlignTextToFramePadding(); ImGui::TextColored(s.net() < 0 ? p.red : p.text, "%s", Format::signedMoney(s.net()).c_str());
            }
            ImGui::EndTable();
        }
        Widgets::endCard();

        ImGui::SameLine(0, gap);
        Widgets::beginCard("##yearChart", ImVec2(0, px(340)));
        Widgets::cardTitle(ICON_FA_CHART_COLUMN, "Income and expense by year");
        if (ImPlot::BeginPlot("##yearBars", ImVec2(-1, -1), ImPlotFlags_NoMenus | ImPlotFlags_NoBoxSelect |
                                                            ImPlotFlags_NoMouseText | ImPlotFlags_NoInputs)) {
            ImPlot::SetupLegend(ImPlotLocation_NorthEast, ImPlotLegendFlags_Horizontal);
            double top = Widgets::maxAbs(income, n);
            if (Widgets::maxAbs(expense, n) > top) top = Widgets::maxAbs(expense, n);
            Widgets::setupBarAxes(positions, n, labels, top);
            ImPlotSpec in; in.FillColor = p.green; in.LineColor = p.green;
            ImPlotSpec out; out.FillColor = p.red; out.LineColor = p.red;
            ImPlot::PlotBars("Income", income, n, 0.3, -0.16, in);
            ImPlot::PlotBars("Expense", expense, n, 0.3, 0.16, out);
            ImPlot::EndPlot();
        }
        Widgets::endCard();

        delete[] years;
        delete[] income;
        delete[] expense;
        delete[] positions;
        delete[] labels;
    }

    // ---------- Tabs 4 + 5: breakdown by source / category ----------
    void breakdownTab(const FinanceData& data, MasterData::Kind kind) {
        const Theme::Palette& p = Theme::colors();
        bool income = (kind == MasterData::Kind::SOURCE);
        yearSelector(data);
        gapY();

        int n = MasterData::count(data, kind);
        DynamicArray<long long> amounts = income ? AnnualStatistics::incomeBySource(data, selectedYears)
                                                 : AnnualStatistics::expenseByCategory(data, selectedYears);
        if (n == 0 || amounts.size() != n) {
            Widgets::beginCard("##breakdownEmpty");
            if (n == 0) {
                Widgets::emptyState(income ? ICON_FA_HAND_HOLDING_DOLLAR : ICON_FA_TAGS,
                                    income ? "No income sources yet" : "No expense categories yet",
                                    "Add them in Master Data.");
            } else {
                Widgets::emptyState(ICON_FA_CHART_PIE, "No data yet");
                Widgets::todoNote(income ? "AnnualStatistics::incomeBySource()" : "AnnualStatistics::expenseByCategory()");
            }
            Widgets::endCard();
            return;
        }

        long long total = 0;
        for (int i = 0; i < n; i++) total += amounts[i];
        double* values = new double[n];
        const char** labels = new const char*[n];
        DynamicArray<std::string> names;
        for (int i = 0; i < n; i++) names.add(MasterData::nameAt(data, kind, i));
        for (int i = 0; i < n; i++) {
            values[i] = (double)amounts[i];
            labels[i] = names[i].c_str();
        }

        float gap = px(18);
        float leftWidth = (ImGui::GetContentRegionAvail().x - gap) * 0.55f;
        Widgets::beginCard("##breakdownTable", ImVec2(leftWidth, px(420)));
        Widgets::cardTitle(ICON_FA_TABLE, income ? "Income by source" : "Expense by category");
        Widgets::mutedText("Years: " + yearsCaption());
        if (ImGui::BeginTable("##breakdown", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH |
                                                ImGuiTableFlags_PadOuterX | ImGuiTableFlags_ScrollY)) {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn(income ? "Source" : "Category", ImGuiTableColumnFlags_WidthStretch, 1.0f);
            ImGui::TableSetupColumn("Share", ImGuiTableColumnFlags_WidthStretch, 1.2f);
            ImGui::TableSetupColumn("Amount", ImGuiTableColumnFlags_WidthFixed, px(130));
            ImGui::TableHeadersRow();
            for (int i = 0; i < n; i++) {
                float share = total > 0 ? (float)amounts[i] / (float)total : 0.0f;
                ImGui::TableNextRow(ImGuiTableRowFlags_None, px(36));
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                Widgets::colorDot(seriesColor(i));
                ImGui::SameLine();
                ImGui::TextUnformatted(names[i].c_str());
                ImGui::TableNextColumn();
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + px(9));
                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, seriesColor(i));
                char overlay[16];
                std::snprintf(overlay, sizeof(overlay), "%.0f%%", share * 100.0f);
                ImGui::ProgressBar(share, ImVec2(-1, px(14)), overlay);
                ImGui::PopStyleColor();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted(Format::money(amounts[i]).c_str());
            }
            ImGui::TableNextRow(ImGuiTableRowFlags_None, px(36));
            ImGui::PushFont(Theme::boldFont(), 0.0f);
            ImGui::TableNextColumn(); ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("Total");
            ImGui::TableNextColumn();
            ImGui::TableNextColumn(); ImGui::AlignTextToFramePadding();
            ImGui::TextColored(income ? p.green : p.red, "%s", Format::money(total).c_str());
            ImGui::PopFont();
            ImGui::EndTable();
        }
        Widgets::endCard();

        ImGui::SameLine(0, gap);
        Widgets::beginCard("##breakdownPie", ImVec2(0, px(420)));
        Widgets::cardTitle(ICON_FA_CHART_PIE, "Share");
        pieChart("##breakdownPieChart", labels, values, n, -1);
        Widgets::endCard();

        delete[] values;
        delete[] labels;
    }
}

namespace Pages {

void statistics(Gui::AppState& app) {
    Widgets::pageHeader("Statistics & Reports", "Totals by period, by wallet and by year");
    ImGui::Spacing();
    const char* tabs[] = {
        ICON_FA_CALENDAR_DAYS "  Date range",
        ICON_FA_WALLET "  By wallet",
        ICON_FA_CHART_COLUMN "  Annual overview",
        ICON_FA_HAND_HOLDING_DOLLAR "  Income by source",
        ICON_FA_TAGS "  Expense by category"
    };
    Widgets::segmented("statTabs", tabs, 5, tab);
    gapY();

    switch (tab) {
        case 0: dateRangeTab(app.data); break;
        case 1: walletTab(app.data); break;
        case 2: annualOverviewTab(app.data); break;
        case 3: breakdownTab(app.data, MasterData::Kind::SOURCE); break;
        case 4: breakdownTab(app.data, MasterData::Kind::CATEGORY); break;
    }
}

}
