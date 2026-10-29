#include "gui/Pages.h"

#include "imgui.h"
#include "IconsFontAwesome6.h"
#include "gui/Theme.h"
#include "gui/Widgets.h"
#include "Format.h"
#include "MasterData.h"
#include "Recurring.h"

using Theme::px;

namespace {
    struct Form {
        bool requestOpen = false;
        bool editing = false;
        int index = -1;
        bool isIncome = false;
        std::string refId;
        long long amount = 0;
        std::string walletId;
        std::string description;
        Date startDate;
        bool hasEndDate = false;
        Date endDate;
        std::string error;
    };
    Form form;
    int pendingDelete = -1;

    std::string refName(const FinanceData& data, const RecurringTransaction& r) {
        return r.getType() == TransactionType::INCOME ? MasterData::getSourceName(data, r.getRefId())
                                                      : MasterData::getCategoryName(data, r.getRefId());
    }

    void openAdd() {
        form = Form();
        form.startDate = Date::today();
        form.endDate = Date::today();
        form.requestOpen = true;
    }

    void openEdit(const FinanceData& data, int index) {
        const RecurringTransaction& r = data.recurrings[index];
        form = Form();
        form.editing = true;
        form.index = index;
        form.isIncome = r.getType() == TransactionType::INCOME;
        form.refId = r.getRefId();
        form.amount = r.getAmount();
        form.walletId = r.getWalletId();
        form.description = r.getDescription();
        form.startDate = r.getStartDate();
        form.hasEndDate = r.getHasEndDate();
        form.endDate = r.getHasEndDate() ? r.getEndDate() : r.getStartDate();
        form.requestOpen = true;
    }

    void formDialog(Gui::AppState& app) {
        if (form.requestOpen) {
            ImGui::OpenPopup("###recurringDialog");
            form.requestOpen = false;
        }
        std::string title = std::string(form.editing ? "Edit recurring transaction" : "New recurring transaction")
                            + "###recurringDialog";
        if (!Widgets::beginModal(title.c_str(), 520)) return;

        const Theme::Palette& p = Theme::colors();
        FinanceData& data = app.data;
        Widgets::mutedText("Added automatically once a month when the app starts.");
        ImGui::Spacing();

        if (!form.editing) {
            const char* types[] = { ICON_FA_ARROW_DOWN "  Income", ICON_FA_ARROW_UP "  Expense" };
            int type = form.isIncome ? 0 : 1;
            if (Widgets::segmented("type", types, 2, type)) {
                form.isIncome = (type == 0);
                form.refId = "";
            }
            ImGui::Spacing();
        }
        MasterData::Kind kind = form.isIncome ? MasterData::Kind::SOURCE : MasterData::Kind::CATEGORY;
        Widgets::fieldLabel(form.isIncome ? "INCOME SOURCE" : "EXPENSE CATEGORY");
        Widgets::masterCombo("##ref", data, kind, form.refId, form.isIncome ? "Select a source..." : "Select a category...");
        Widgets::fieldLabel("AMOUNT PER MONTH (VND)");
        Widgets::amountInput("amount", form.amount);
        Widgets::fieldLabel("WALLET");
        Widgets::masterCombo("##wallet", data, MasterData::Kind::WALLET, form.walletId, "Select a wallet...");
        Widgets::fieldLabel("DESCRIPTION");
        Widgets::textInput("##description", form.description, "e.g. Monthly salary, Netflix, Rent");

        float half = (ImGui::GetContentRegionAvail().x - px(12)) * 0.5f;
        ImGui::BeginGroup();
        Widgets::fieldLabel("START DATE");
        Widgets::dateInput("start", form.startDate, half);
        ImGui::EndGroup();
        ImGui::SameLine(0, px(12));
        ImGui::BeginGroup();
        Widgets::fieldLabel("END DATE");
        if (form.hasEndDate) {
            Widgets::dateInput("end", form.endDate, half);
        } else {
            ImGui::BeginDisabled();
            ImGui::Button(ICON_FA_INFINITY "   No end date", ImVec2(half, 0));
            ImGui::EndDisabled();
        }
        ImGui::EndGroup();
        ImGui::Checkbox("Stop on an end date", &form.hasEndDate);

        ImGui::Spacing();
        Widgets::formError(form.error);
        ImGui::Spacing();

        const char* saveLabel = ICON_FA_CHECK "  Save";
        Widgets::alignRight(Widgets::buttonWidth("Cancel") + Widgets::buttonWidth(saveLabel) + px(10));
        if (Widgets::ghostButton("Cancel")) ImGui::CloseCurrentPopup();
        ImGui::SameLine();
        if (Widgets::button(saveLabel, p.blue)) {
            form.error = "";
            if (form.refId.empty()) form.error = form.isIncome ? "Please choose an income source." : "Please choose an expense category.";
            else if (form.amount <= 0) form.error = "Amount must be greater than 0.";
            else if (form.walletId.empty()) form.error = "Please choose a wallet.";
            else if (form.hasEndDate && form.endDate < form.startDate) form.error = "End date must be on or after the start date.";

            if (form.error.empty()) {
                RecurringTransaction item(form.editing ? data.recurrings[form.index].getId() : "",
                                          form.isIncome ? TransactionType::INCOME : TransactionType::EXPENSE,
                                          form.refId, form.amount, form.walletId, form.description,
                                          form.startDate, form.hasEndDate, form.endDate);
                if (form.editing) {
                    if (Recurring::updateRecurring(data, form.index, item)) {
                        ImGui::CloseCurrentPopup();
                        Gui::finishAction(app, "Recurring transaction updated.");
                    } else {
                        form.error = "Recurring::updateRecurring() is not implemented yet.";
                    }
                } else {
                    std::string id = Recurring::addRecurring(data, item);
                    if (!id.empty()) {
                        ImGui::CloseCurrentPopup();
                        Gui::finishAction(app, "Recurring transaction " + id + " created.");
                    } else {
                        form.error = "Recurring::addRecurring() is not implemented yet.";
                    }
                }
            }
        }
        Widgets::endModal();
    }

    void deleteDialog(Gui::AppState& app) {
        if (!Widgets::beginModal("Delete recurring transaction###deleteRecurring", 460)) return;
        const Theme::Palette& p = Theme::colors();
        ImGui::TextWrapped("It will no longer be added every month. Transactions that were already created are kept.");
        ImGui::Spacing();
        Widgets::alignRight(Widgets::buttonWidth("Cancel") + Widgets::buttonWidth(ICON_FA_TRASH "  Delete") + px(10));
        if (Widgets::ghostButton("Cancel")) ImGui::CloseCurrentPopup();
        ImGui::SameLine();
        if (Widgets::button(ICON_FA_TRASH "  Delete", p.red)) {
            ImGui::CloseCurrentPopup();
            if (Recurring::removeRecurring(app.data, pendingDelete)) {
                Gui::finishAction(app, "Recurring transaction deleted.");
            } else {
                Widgets::toast("Recurring::removeRecurring() is not implemented yet.", true);
            }
        }
        Widgets::endModal();
    }
}

namespace Pages {

void recurring(Gui::AppState& app) {
    const Theme::Palette& p = Theme::colors();
    FinanceData& data = app.data;

    const char* addLabel = ICON_FA_PLUS "  New recurring";
    Widgets::pageHeader("Recurring transactions", "Monthly income and expenses that are added for you",
                        Widgets::buttonWidth(addLabel));
    if (Widgets::button(addLabel, p.blue)) openAdd();
    Widgets::headerEnd();
    ImGui::Spacing();

    // Info banner
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(p.blue.x, p.blue.y, p.blue.z, 0.10f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(p.blue.x, p.blue.y, p.blue.z, 0.35f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(px(16), px(12)));
    ImGui::BeginChild("##info", ImVec2(0, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY |
                                              ImGuiChildFlags_AlwaysUseWindowPadding);
    ImGui::TextColored(p.blue, ICON_FA_CIRCLE_INFO);
    ImGui::SameLine();
    ImGui::TextWrapped("Each time the app starts, every active item is added once for the current month. "
                       "It is never added twice in the same month.");
    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);
    ImGui::Spacing();

    float height = ImGui::GetContentRegionAvail().y;
    if (height < px(300)) height = px(300);
    Widgets::beginCard("##recurringTable", ImVec2(0, height));
    if (data.recurrings.isEmpty()) {
        Widgets::emptyState(ICON_FA_ARROWS_ROTATE, "No recurring transactions",
                            "Create one for your salary, rent or subscriptions.");
    } else {
        Date today = Date::today();
        ImGuiTableFlags flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_ScrollY |
                                ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_PadOuterX;
        if (ImGui::BeginTable("##recurrings", 9, flags)) {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, px(64));
            ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, px(86));
            ImGui::TableSetupColumn("Source / Category", ImGuiTableColumnFlags_WidthStretch, 1.1f);
            ImGui::TableSetupColumn("Description", ImGuiTableColumnFlags_WidthStretch, 1.4f);
            ImGui::TableSetupColumn("Wallet", ImGuiTableColumnFlags_WidthStretch, 1.0f);
            ImGui::TableSetupColumn("Amount / month", ImGuiTableColumnFlags_WidthFixed, px(130));
            ImGui::TableSetupColumn("Period", ImGuiTableColumnFlags_WidthFixed, px(190));
            ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthFixed, px(90));
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, px(72));
            ImGui::TableHeadersRow();

            for (int i = 0; i < data.recurrings.size(); i++) {
                const RecurringTransaction& r = data.recurrings[i];
                bool income = r.getType() == TransactionType::INCOME;
                ImGui::PushID(i);
                ImGui::TableNextRow(ImGuiTableRowFlags_None, px(40));

                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                Widgets::mutedText(r.getId());
                ImGui::TableNextColumn();
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + px(5));
                Widgets::badge(income ? "Income" : "Expense", income ? p.green : p.red);
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted(refName(data, r).c_str());
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::TextColored(r.getDescription().empty() ? p.textMuted : p.text, "%s",
                                   r.getDescription().empty() ? "-" : r.getDescription().c_str());
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted(MasterData::getWalletName(data, r.getWalletId()).c_str());

                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                std::string amount = Format::signedMoney(income ? r.getAmount() : -r.getAmount());
                float w = ImGui::CalcTextSize(amount.c_str()).x;
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - w);
                ImGui::PushFont(Theme::boldFont(), 0.0f);
                ImGui::TextColored(income ? p.green : p.red, "%s", amount.c_str());
                ImGui::PopFont();

                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                std::string period = r.getStartDate().toString() + "  " ICON_FA_ARROW_RIGHT "  "
                                     + (r.getHasEndDate() ? r.getEndDate().toString() : std::string(ICON_FA_INFINITY));
                ImGui::TextUnformatted(period.c_str());

                ImGui::TableNextColumn();
                bool ended = r.getHasEndDate() && r.getEndDate() < today;
                bool scheduled = today < r.getStartDate();
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + px(5));
                Widgets::badge(ended ? "Ended" : (scheduled ? "Scheduled" : "Active"),
                               ended ? p.textMuted : (scheduled ? p.blue : p.green));

                ImGui::TableNextColumn();
                if (Widgets::iconButton("edit", ICON_FA_PEN, p.blue, "Edit")) openEdit(data, i);
                ImGui::SameLine(0, px(2));
                if (Widgets::iconButton("delete", ICON_FA_TRASH, p.red, "Delete")) {
                    pendingDelete = i;
                    ImGui::OpenPopup("###deleteRecurring");
                }
                if (ImGui::IsPopupOpen("###deleteRecurring")) deleteDialog(app);
                ImGui::PopID();
            }
            ImGui::EndTable();
        }
    }
    Widgets::endCard();
    formDialog(app);
}

}
