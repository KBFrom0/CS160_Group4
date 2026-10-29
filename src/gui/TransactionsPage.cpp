#include "gui/Pages.h"

#include <cctype>
#include "imgui.h"
#include "imgui_stdlib.h"
#include "IconsFontAwesome6.h"
#include "gui/Theme.h"
#include "gui/Widgets.h"
#include "Format.h"
#include "MasterData.h"

using Theme::px;

namespace {
    // ---- Add / edit dialog state ----
    struct Form {
        bool requestOpen = false;
        bool editing = false;
        Transactions::TransactionRef ref;
        Pages::TransactionView view;
        std::string error;
    };
    Form form;

    // ---- List page state ----
    std::string search;
    int typeFilter = 0;          // 0 all, 1 income, 2 expense
    std::string walletFilter;    // "" = all wallets
    Transactions::TransactionRef pendingDelete;

    std::string lower(const std::string& text) {
        std::string result = text;
        for (size_t i = 0; i < result.size(); i++) result[i] = (char)std::tolower((unsigned char)result[i]);
        return result;
    }

    bool matches(const FinanceData& data, const Pages::TransactionView& t) {
        if (typeFilter == 1 && !t.isIncome) return false;
        if (typeFilter == 2 && t.isIncome) return false;
        if (!walletFilter.empty() && t.walletId != walletFilter) return false;
        if (search.empty()) return true;
        std::string needle = lower(search);
        std::string haystack = lower(t.description + " " + Pages::refNameOf(data, t) + " "
                                     + MasterData::getWalletName(data, t.walletId) + " " + t.date.toString());
        return haystack.find(needle) != std::string::npos;
    }

    void deleteDialog(Gui::AppState& app) {
        if (!Widgets::beginModal("Delete transaction###deleteTx", 440)) return;
        const Theme::Palette& p = Theme::colors();
        Pages::TransactionView t = Pages::viewOf(app.data, pendingDelete);
        ImGui::TextWrapped("This transaction will be removed permanently. Wallet balances will be updated.");
        ImGui::Spacing();
        Widgets::beginCard("##deletePreview");
        ImGui::TextColored(t.isIncome ? p.green : p.red, "%s",
                           Format::signedMoney(t.isIncome ? t.amount : -t.amount).c_str());
        Widgets::mutedText(t.date.toString() + "  ·  " + Pages::refNameOf(app.data, t)
                           + (t.description.empty() ? "" : "  ·  " + t.description));
        Widgets::endCard();
        ImGui::Spacing();

        Widgets::alignRight(Widgets::buttonWidth("Cancel") + Widgets::buttonWidth(ICON_FA_TRASH "  Delete") + px(10));
        if (Widgets::ghostButton("Cancel")) ImGui::CloseCurrentPopup();
        ImGui::SameLine();
        if (Widgets::button(ICON_FA_TRASH "  Delete", p.red)) {
            ImGui::CloseCurrentPopup();
            if (Transactions::deleteTransaction(app.data, pendingDelete)) {
                Gui::finishAction(app, "Transaction deleted.");
            } else {
                Widgets::toast("Transactions::deleteTransaction() is not implemented yet.", true);
            }
        }
        Widgets::endModal();
    }
}

namespace Pages {

TransactionView viewOf(const FinanceData& data, const Transactions::TransactionRef& ref) {
    TransactionView v;
    v.isIncome = ref.isIncome;
    if (ref.isIncome && ref.index >= 0 && ref.index < data.incomes.size()) {
        const Income& t = data.incomes[ref.index];
        v.id = t.getId(); v.date = t.getDate(); v.refId = t.getSourceId(); v.amount = t.getAmount();
        v.walletId = t.getWalletId(); v.description = t.getDescription(); v.recurringId = t.getRecurringId();
    } else if (!ref.isIncome && ref.index >= 0 && ref.index < data.expenses.size()) {
        const Expense& t = data.expenses[ref.index];
        v.id = t.getId(); v.date = t.getDate(); v.refId = t.getCategoryId(); v.amount = t.getAmount();
        v.walletId = t.getWalletId(); v.description = t.getDescription(); v.recurringId = t.getRecurringId();
    }
    return v;
}

std::string refNameOf(const FinanceData& data, const TransactionView& view) {
    return view.isIncome ? MasterData::getSourceName(data, view.refId)
                         : MasterData::getCategoryName(data, view.refId);
}

void openAddTransaction(bool isIncome) {
    form = Form();
    form.view.isIncome = isIncome;
    form.view.date = Date::today();
    form.requestOpen = true;
}

void openEditTransaction(const Gui::AppState& app, const Transactions::TransactionRef& ref) {
    form = Form();
    form.editing = true;
    form.ref = ref;
    form.view = viewOf(app.data, ref);
    form.requestOpen = true;
}

void transactionDialog(Gui::AppState& app) {
    if (form.requestOpen) {
        ImGui::OpenPopup("###transactionDialog");
        form.requestOpen = false;
    }
    TransactionView& v = form.view;
    std::string title = std::string(form.editing ? "Edit " : "Add ") + (v.isIncome ? "income" : "expense")
                        + "###transactionDialog";
    if (!Widgets::beginModal(title.c_str(), 480)) return;

    const Theme::Palette& p = Theme::colors();
    FinanceData& data = app.data;
    MasterData::Kind refKind = v.isIncome ? MasterData::Kind::SOURCE : MasterData::Kind::CATEGORY;

    if (!form.editing) {
        const char* types[] = { ICON_FA_ARROW_DOWN "  Income", ICON_FA_ARROW_UP "  Expense" };
        int type = v.isIncome ? 0 : 1;
        if (Widgets::segmented("type", types, 2, type)) {
            v.isIncome = (type == 0);
            v.refId = "";
            form.error = "";
        }
        ImGui::Spacing();
    }

    Widgets::fieldLabel("DATE");
    Widgets::dateInput("date", v.date);
    Widgets::fieldLabel(v.isIncome ? "INCOME SOURCE" : "EXPENSE CATEGORY");
    Widgets::masterCombo("##ref", data, refKind, v.refId, v.isIncome ? "Select a source..." : "Select a category...");
    Widgets::fieldLabel("AMOUNT (VND)");
    Widgets::amountInput("amount", v.amount);
    Widgets::fieldLabel("WALLET");
    Widgets::masterCombo("##wallet", data, MasterData::Kind::WALLET, v.walletId, "Select a wallet...");
    Widgets::fieldLabel("DESCRIPTION");
    Widgets::textInput("##description", v.description, "Optional note");
    ImGui::Spacing();
    Widgets::formError(form.error);
    ImGui::Spacing();

    const char* saveLabel = form.editing ? ICON_FA_CHECK "  Save changes" : ICON_FA_CHECK "  Save";
    Widgets::alignRight(Widgets::buttonWidth("Cancel") + Widgets::buttonWidth(saveLabel) + px(10));
    if (Widgets::ghostButton("Cancel")) ImGui::CloseCurrentPopup();
    ImGui::SameLine();
    if (Widgets::button(saveLabel, v.isIncome ? p.green : p.blue)) {
        form.error = "";
        if (v.refId.empty()) form.error = v.isIncome ? "Please choose an income source." : "Please choose an expense category.";
        else if (v.amount <= 0) form.error = "Amount must be greater than 0.";
        else if (v.walletId.empty()) form.error = "Please choose a wallet.";

        if (form.error.empty()) {
            if (form.editing) {
                bool ok = v.isIncome
                    ? Transactions::updateIncome(data, form.ref.index,
                          Income(v.id, v.date, v.refId, v.amount, v.walletId, v.description, v.recurringId))
                    : Transactions::updateExpense(data, form.ref.index,
                          Expense(v.id, v.date, v.refId, v.amount, v.walletId, v.description, v.recurringId));
                if (ok) {
                    ImGui::CloseCurrentPopup();
                    Gui::finishAction(app, "Changes saved.");
                } else {
                    form.error = std::string("Transactions::") + (v.isIncome ? "updateIncome" : "updateExpense")
                                 + "() is not implemented yet.";
                }
            } else {
                std::string id = v.isIncome
                    ? Transactions::addIncomeRecord(data, v.date, v.refId, v.amount, v.walletId, v.description)
                    : Transactions::addExpenseRecord(data, v.date, v.refId, v.amount, v.walletId, v.description);
                if (!id.empty()) {
                    ImGui::CloseCurrentPopup();
                    Gui::finishAction(app, std::string(v.isIncome ? "Income " : "Expense ") + id + " saved.");
                } else {
                    form.error = std::string("Transactions::") + (v.isIncome ? "addIncomeRecord" : "addExpenseRecord")
                                 + "() is not implemented yet.";
                }
            }
        }
    }
    Widgets::endModal();
}

void transactions(Gui::AppState& app) {
    const Theme::Palette& p = Theme::colors();
    FinanceData& data = app.data;

    const char* addIncome = ICON_FA_PLUS "  Add income";
    const char* addExpense = ICON_FA_MINUS "  Add expense";
    Widgets::pageHeader("Transactions", "Every income and expense, newest first",
                        Widgets::buttonWidth(addIncome) + Widgets::buttonWidth(addExpense) + px(10));
    if (Widgets::button(addIncome, p.green)) openAddTransaction(true);
    ImGui::SameLine();
    if (Widgets::button(addExpense, p.red)) openAddTransaction(false);
    Widgets::headerEnd();
    ImGui::Spacing();

    // Filters
    Widgets::beginCard("##filters");
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.42f);
    ImGui::InputTextWithHint("##search", ICON_FA_MAGNIFYING_GLASS "   Search description, source, category, wallet...", &search);
    ImGui::SameLine(0, px(16));
    const char* types[] = { "All", "Income", "Expense" };
    Widgets::segmented("typeFilter", types, 3, typeFilter);
    ImGui::SameLine(0, px(16));
    Widgets::masterFilter("##walletFilter", data, MasterData::Kind::WALLET, walletFilter,
                          ICON_FA_WALLET "  All wallets", px(200));
    if (!search.empty() || typeFilter != 0 || !walletFilter.empty()) {
        ImGui::SameLine();
        if (Widgets::ghostButton(ICON_FA_XMARK "  Clear")) {
            search = "";
            typeFilter = 0;
            walletFilter = "";
        }
    }
    Widgets::endCard();
    ImGui::Spacing();

    // Table
    DynamicArray<Transactions::TransactionRef> refs = Transactions::getSortedTransactions(data);
    float height = ImGui::GetContentRegionAvail().y;
    if (height < px(320)) height = px(320);
    Widgets::beginCard("##txTable", ImVec2(0, height));
    if (refs.isEmpty()) {
        Widgets::emptyState(ICON_FA_RECEIPT, "No transactions yet", "Click \"Add income\" or \"Add expense\" to create one.");
        if (!data.incomes.isEmpty() || !data.expenses.isEmpty()) {
            Widgets::todoNote("Transactions::getSortedTransactions()  (" +
                              std::to_string(data.incomes.size() + data.expenses.size()) + " transactions in the data)");
        }
        Widgets::endCard();
        return;
    }

    int shown = 0;
    ImGuiTableFlags flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_ScrollY |
                            ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_PadOuterX;
    if (ImGui::BeginTable("##transactions", 7, flags, ImVec2(0, -ImGui::GetFrameHeightWithSpacing()))) {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("Date", ImGuiTableColumnFlags_WidthFixed, px(96));
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, px(90));
        ImGui::TableSetupColumn("Source / Category", ImGuiTableColumnFlags_WidthStretch, 1.2f);
        ImGui::TableSetupColumn("Wallet", ImGuiTableColumnFlags_WidthStretch, 1.0f);
        ImGui::TableSetupColumn("Description", ImGuiTableColumnFlags_WidthStretch, 1.6f);
        ImGui::TableSetupColumn("Amount", ImGuiTableColumnFlags_WidthFixed, px(130));
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, px(72));
        ImGui::TableHeadersRow();

        for (int i = 0; i < refs.size(); i++) {
            TransactionView t = viewOf(data, refs[i]);
            if (!matches(data, t)) continue;
            shown++;
            ImGui::PushID(i);
            ImGui::TableNextRow(ImGuiTableRowFlags_None, px(40));

            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted(t.date.toString().c_str());

            ImGui::TableNextColumn();
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + px(5));
            Widgets::badge(t.isIncome ? "Income" : "Expense", t.isIncome ? p.green : p.red);

            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted(refNameOf(data, t).c_str());
            if (!t.recurringId.empty()) {
                ImGui::SameLine();
                ImGui::TextColored(p.textMuted, ICON_FA_ARROWS_ROTATE);
                ImGui::SetItemTooltip("Created by recurring transaction %s", t.recurringId.c_str());
            }

            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted(MasterData::getWalletName(data, t.walletId).c_str());

            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::TextColored(t.description.empty() ? p.textMuted : p.text, "%s",
                               t.description.empty() ? "-" : t.description.c_str());

            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            std::string amount = Format::signedMoney(t.isIncome ? t.amount : -t.amount);
            float w = ImGui::CalcTextSize(amount.c_str()).x;
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - w);
            ImGui::PushFont(Theme::boldFont(), 0.0f);
            ImGui::TextColored(t.isIncome ? p.green : p.red, "%s", amount.c_str());
            ImGui::PopFont();

            ImGui::TableNextColumn();
            if (Widgets::iconButton("edit", ICON_FA_PEN, p.blue, "Edit")) openEditTransaction(app, refs[i]);
            ImGui::SameLine(0, px(2));
            if (Widgets::iconButton("delete", ICON_FA_TRASH, p.red, "Delete")) {
                pendingDelete = refs[i];
                ImGui::OpenPopup("###deleteTx");
            }
            if (ImGui::IsPopupOpen("###deleteTx")) deleteDialog(app);
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    Widgets::mutedText("Showing " + std::to_string(shown) + " of " + std::to_string(refs.size()) + " transactions");
    Widgets::endCard();
}

}
