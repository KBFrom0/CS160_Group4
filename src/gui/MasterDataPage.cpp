#include "gui/Pages.h"

#include "imgui.h"
#include "imgui_stdlib.h"
#include "IconsFontAwesome6.h"
#include "gui/Theme.h"
#include "gui/Widgets.h"
#include "MasterData.h"

using Theme::px;
using MasterData::Kind;

namespace {
    std::string newNames[3];          // text in the "Add" field of each column
    Kind pendingKind = Kind::CATEGORY;
    std::string pendingId;
    std::string pendingName;
    std::string renameText;
    std::string renameError;
    bool requestRename = false;
    bool requestDelete = false;

    int kindIndex(Kind kind) {
        return kind == Kind::CATEGORY ? 0 : (kind == Kind::SOURCE ? 1 : 2);
    }

    const char* singular(Kind kind) {
        return kind == Kind::CATEGORY ? "category" : (kind == Kind::SOURCE ? "income source" : "wallet");
    }

    bool inUse(const FinanceData& data, Kind kind, const std::string& id) {
        switch (kind) {
            case Kind::CATEGORY: return MasterData::isCategoryInUse(data, id);
            case Kind::SOURCE:   return MasterData::isSourceInUse(data, id);
            default:             return MasterData::isWalletInUse(data, id);
        }
    }

    void addItem(Gui::AppState& app, Kind kind) {
        std::string& name = newNames[kindIndex(kind)];
        std::string message;
        if (MasterData::addItem(app.data, kind, name, message)) {
            std::string added = name;
            name = "";
            Gui::finishAction(app, message.empty() ? "Added \"" + added + "\"." : message);
        } else {
            Widgets::toast(message.empty() ? "Could not add this item." : message, true);
        }
    }

    void column(Gui::AppState& app, Kind kind, const char* icon, const ImVec4& color,
                const char* title, const char* hint, const ImVec2& size) {
        const Theme::Palette& p = Theme::colors();
        FinanceData& data = app.data;
        int k = kindIndex(kind);
        ImGui::PushID(k);
        Widgets::beginCard("##column", size);

        // Header: colored icon, title, count
        float r = px(18);
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImDrawList* draw = ImGui::GetWindowDrawList();
        draw->AddRectFilled(pos, ImVec2(pos.x + r * 2, pos.y + r * 2), Theme::toU32(color, 0.15f), px(8));
        ImVec2 iconSize = ImGui::CalcTextSize(icon);
        draw->AddText(ImVec2(pos.x + r - iconSize.x * 0.5f, pos.y + r - iconSize.y * 0.5f), Theme::toU32(color), icon);
        ImGui::Dummy(ImVec2(r * 2, r * 2));
        ImGui::SameLine(0, px(12));
        ImGui::BeginGroup();
        ImGui::PushFont(Theme::boldFont(), Theme::FONT_LARGE);
        ImGui::TextUnformatted(title);
        ImGui::PopFont();
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - px(6));
        ImGui::PushFont(nullptr, Theme::FONT_SMALL);
        Widgets::mutedText(hint);
        ImGui::PopFont();
        ImGui::EndGroup();
        std::string count = std::to_string(MasterData::count(data, kind));
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(count.c_str()).x - px(16));
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + px(6));
        Widgets::badge(count.c_str(), color);
        ImGui::Spacing();

        // Add row
        const char* addLabel = ICON_FA_PLUS "  Add";
        float addWidth = Widgets::buttonWidth(addLabel);
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - addWidth - ImGui::GetStyle().ItemSpacing.x);
        std::string placeholder = std::string("New ") + singular(kind) + " name";
        bool enter = ImGui::InputTextWithHint("##new", placeholder.c_str(), &newNames[k], ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::SameLine();
        if (Widgets::button(addLabel, color) || enter) addItem(app, kind);
        ImGui::Spacing();

        // List
        int n = MasterData::count(data, kind);
        if (n == 0) {
            Widgets::emptyState(icon, std::string("No ") + singular(kind) + " yet", "Type a name above and press Add.");
        } else if (ImGui::BeginTable("##items", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH |
                                                   ImGuiTableFlags_ScrollY | ImGuiTableFlags_PadOuterX)) {
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, px(64));
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, px(68));
            for (int i = 0; i < n; i++) {
                std::string id = MasterData::idAt(data, kind, i);
                std::string name = MasterData::nameAt(data, kind, i);
                ImGui::PushID(i);
                ImGui::TableNextRow(ImGuiTableRowFlags_None, px(38));
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted(name.c_str());
                if (inUse(data, kind, id)) {
                    ImGui::SameLine();
                    ImGui::TextColored(p.textMuted, ICON_FA_LINK);
                    ImGui::SetItemTooltip("Used by transactions - cannot be deleted");
                }
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::PushFont(nullptr, Theme::FONT_SMALL);
                Widgets::mutedText(id);
                ImGui::PopFont();
                ImGui::TableNextColumn();
                if (Widgets::iconButton("rename", ICON_FA_PEN, p.blue, "Rename")) {
                    pendingKind = kind; pendingId = id; pendingName = name;
                    renameText = name; renameError = "";
                    requestRename = true;
                }
                ImGui::SameLine(0, px(2));
                if (Widgets::iconButton("delete", ICON_FA_TRASH, p.red, "Delete")) {
                    pendingKind = kind; pendingId = id; pendingName = name;
                    requestDelete = true;
                }
                ImGui::PopID();
            }
            ImGui::EndTable();
        }
        Widgets::endCard();
        ImGui::PopID();
    }

    void renameDialog(Gui::AppState& app) {
        if (requestRename) {
            ImGui::OpenPopup("###renameItem");
            requestRename = false;
        }
        std::string title = std::string("Rename ") + singular(pendingKind) + "###renameItem";
        if (!Widgets::beginModal(title.c_str(), 440)) return;
        Widgets::fieldLabel("NAME");
        if (ImGui::IsWindowAppearing()) ImGui::SetKeyboardFocusHere();
        Widgets::textInput("##rename", renameText, "Name");
        Widgets::formError(renameError);
        ImGui::Spacing();
        const char* saveLabel = ICON_FA_CHECK "  Save";
        Widgets::alignRight(Widgets::buttonWidth("Cancel") + Widgets::buttonWidth(saveLabel) + px(10));
        if (Widgets::ghostButton("Cancel")) ImGui::CloseCurrentPopup();
        ImGui::SameLine();
        if (Widgets::button(saveLabel, Theme::colors().blue)) {
            std::string message;
            if (MasterData::renameItem(app.data, pendingKind, pendingId, renameText, message)) {
                ImGui::CloseCurrentPopup();
                Gui::finishAction(app, message.empty() ? "Renamed to \"" + renameText + "\"." : message);
            } else {
                renameError = message.empty() ? "Could not rename this item." : message;
            }
        }
        Widgets::endModal();
    }

    void deleteDialog(Gui::AppState& app) {
        if (requestDelete) {
            ImGui::OpenPopup("###deleteItem");
            requestDelete = false;
        }
        std::string title = std::string("Delete ") + singular(pendingKind) + "###deleteItem";
        if (!Widgets::beginModal(title.c_str(), 440)) return;
        const Theme::Palette& p = Theme::colors();
        bool used = inUse(app.data, pendingKind, pendingId);
        if (used) {
            ImGui::TextColored(p.orange, ICON_FA_TRIANGLE_EXCLAMATION);
            ImGui::SameLine();
            ImGui::TextWrapped("\"%s\" is used by transactions or recurring items, so it cannot be deleted.",
                               pendingName.c_str());
        } else {
            ImGui::TextWrapped("Delete \"%s\"? This cannot be undone.", pendingName.c_str());
        }
        ImGui::Spacing();
        const char* deleteLabel = ICON_FA_TRASH "  Delete";
        Widgets::alignRight(Widgets::buttonWidth("Cancel") + Widgets::buttonWidth(deleteLabel) + px(10));
        if (Widgets::ghostButton("Cancel")) ImGui::CloseCurrentPopup();
        ImGui::SameLine();
        ImGui::BeginDisabled(used);
        if (Widgets::button(deleteLabel, p.red)) {
            ImGui::CloseCurrentPopup();
            std::string message;
            if (MasterData::removeItem(app.data, pendingKind, pendingId, message)) {
                Gui::finishAction(app, message.empty() ? "Deleted \"" + pendingName + "\"." : message);
            } else {
                Widgets::toast(message.empty() ? "Could not delete this item." : message, true);
            }
        }
        ImGui::EndDisabled();
        Widgets::endModal();
    }
}

namespace Pages {

void masterData(Gui::AppState& app) {
    const Theme::Palette& p = Theme::colors();
    Widgets::pageHeader("Master Data", "Categories, income sources and wallets used when entering transactions");
    ImGui::Spacing();

    float gap = px(18);
    float avail = ImGui::GetContentRegionAvail().x;
    float height = ImGui::GetContentRegionAvail().y;
    if (height < px(420)) height = px(420);
    ImVec2 size((avail - gap * 2) / 3, height);

    column(app, Kind::CATEGORY, ICON_FA_TAGS, p.orange, "Expense categories", "Food, Rent, Transportation...", size);
    ImGui::SameLine(0, gap);
    column(app, Kind::SOURCE, ICON_FA_HAND_HOLDING_DOLLAR, p.green, "Income sources", "Salary, Bonus, Interest...", size);
    ImGui::SameLine(0, gap);
    column(app, Kind::WALLET, ICON_FA_WALLET, p.blue, "Wallets", "Cash, Bank account, Card...", size);

    renameDialog(app);
    deleteDialog(app);
}

}
