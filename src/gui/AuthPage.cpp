// Get Started, Log in and Create account screens (shown until someone logs in).
// Left half: gradient with the app logo and the team photos on an orbit.
// Right half: the welcome text and buttons, or the log in / register form.
#include "gui/Pages.h"

#include <cmath>
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include "gui/Images.h"
#include "gui/Theme.h"
#include "gui/Widgets.h"
#include "Auth.h"

using Theme::px;

namespace {
    const float PI = 3.14159265f;
    const int MEMBER_COUNT = 4;
    Images::Texture members[MEMBER_COUNT];
    bool imagesLoaded = false;

    // Form state
    std::string loginUser, loginPassword, loginError;
    std::string regNickname, regUser, regPassword, regConfirm, regError;
    bool focusFirstField = false;
    float formHeight[3] = { 0, 0, 0 };   // measured last frame, used to center the form

    void loadImages() {
        if (imagesLoaded) return;
        imagesLoaded = true;
        for (int i = 0; i < MEMBER_COUNT; i++) {
            std::string path = "assets/images/member" + std::to_string(i + 1) + ".png";
            Images::load(path.c_str(), members[i]);
        }
    }

    void goTo(Gui::AppState& app, Gui::AuthScreen screen) {
        app.authScreen = screen;
        loginError = "";
        regError = "";
        loginPassword = "";
        regPassword = "";
        regConfirm = "";
        focusFirstField = true;
    }

    // ---------- Left half ----------
    void heroPanel(const ImVec2& min, const ImVec2& max) {
        ImDrawList* draw = ImGui::GetWindowDrawList();
        draw->AddRectFilledMultiColor(min, max, IM_COL32(44, 95, 214, 255), IM_COL32(91, 63, 208, 255),
                                      IM_COL32(27, 31, 53, 255), IM_COL32(52, 58, 150, 255));
        // Soft light spots
        draw->AddCircleFilled(ImVec2(min.x + (max.x - min.x) * 0.18f, min.y + (max.y - min.y) * 0.15f),
                              (max.x - min.x) * 0.22f, IM_COL32(255, 255, 255, 14), 64);
        draw->AddCircleFilled(ImVec2(min.x + (max.x - min.x) * 0.85f, min.y + (max.y - min.y) * 0.88f),
                              (max.x - min.x) * 0.28f, IM_COL32(255, 255, 255, 10), 64);

        ImVec2 center((min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f - px(20));
        float radius = std::fmin(max.x - min.x, max.y - min.y) * 0.26f;

        // Dashed orbit
        const int dashes = 64;
        for (int i = 0; i < dashes; i += 2) {
            float a0 = 2.0f * PI * i / dashes;
            float a1 = 2.0f * PI * (i + 1) / dashes;
            draw->PathArcTo(center, radius, a0, a1, 6);
            draw->PathStroke(IM_COL32(255, 255, 255, 90), 0, px(1.5f));
        }

        // Logo in the middle
        float logo = px(104);
        ImVec2 logoMin(center.x - logo * 0.5f, center.y - logo * 0.5f);
        draw->AddRectFilled(ImVec2(logoMin.x + px(4), logoMin.y + px(8)),
                            ImVec2(logoMin.x + logo + px(4), logoMin.y + logo + px(8)), IM_COL32(0, 0, 0, 50), px(26));
        draw->AddRectFilled(logoMin, ImVec2(logoMin.x + logo, logoMin.y + logo), IM_COL32_WHITE, px(26));
        ImFont* font = Theme::regularFont();
        float iconSize = px(50);
        ImVec2 iconDim = font->CalcTextSizeA(iconSize, 1000.0f, 0.0f, ICON_FA_WALLET);
        draw->AddText(font, iconSize, ImVec2(center.x - iconDim.x * 0.5f, center.y - iconDim.y * 0.5f),
                      Theme::toU32(Theme::colors().blue), ICON_FA_WALLET);

        // Team photos turning slowly around the logo
        float avatar = px(50);
        float turn = (float)ImGui::GetTime() * 0.12f;
        for (int i = 0; i < MEMBER_COUNT; i++) {
            float angle = -PI * 0.5f + turn + i * (2.0f * PI / MEMBER_COUNT);
            ImVec2 c(center.x + std::cos(angle) * radius, center.y + std::sin(angle) * radius);
            draw->AddCircleFilled(ImVec2(c.x, c.y + px(4)), avatar + px(5), IM_COL32(0, 0, 0, 60), 48);
            draw->AddCircleFilled(c, avatar + px(4), IM_COL32(255, 255, 255, 235), 48);
            if (members[i].id != 0) {
                Images::drawCircle(draw, members[i], c, avatar);
            } else {
                draw->AddCircleFilled(c, avatar, IM_COL32(60, 70, 110, 255), 48);
            }
        }

        // Caption under the orbit
        const char* caption = "OUR TEAM";
        ImFont* bold = Theme::boldFont();
        float captionSize = px(Theme::FONT_SMALL);
        ImVec2 dim = bold->CalcTextSizeA(captionSize, 1000.0f, 0.0f, caption);
        draw->AddText(bold, captionSize, ImVec2(center.x - dim.x * 0.5f, center.y + radius + avatar + px(34)),
                      IM_COL32(255, 255, 255, 200), caption);
    }

    // ---------- Right half helpers ----------
    void title(const char* text, float size) {
        ImGui::PushFont(Theme::boldFont(), size);
        ImGui::TextWrapped("%s", text);
        ImGui::PopFont();
    }

    void subtitle(const std::string& text) {
        ImGui::PushStyleColor(ImGuiCol_Text, Theme::colors().textMuted);
        ImGui::TextWrapped("%s", text.c_str());
        ImGui::PopStyleColor();
    }

    bool linkButton(const char* label) {
        const Theme::Palette& p = Theme::colors();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_Text, p.blue);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        bool clicked = ImGui::Button(label);
        if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(5);
        return clicked;
    }

    void footerLink(const char* text, const char* link, Gui::AppState& app, Gui::AuthScreen target) {
        float width = ImGui::CalcTextSize(text).x + px(6) + ImGui::CalcTextSize(link).x;
        ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - width) * 0.5f);
        Widgets::mutedText(text);
        ImGui::SameLine(0, px(6));
        if (linkButton(link)) goTo(app, target);
    }

    void backButton(Gui::AppState& app) {
        if (Widgets::ghostButton(ICON_FA_ARROW_LEFT "  Back")) goTo(app, Gui::AuthScreen::WELCOME);
        ImGui::Dummy(ImVec2(0, px(4)));
    }

    void focusHere() {
        if (focusFirstField) {
            ImGui::SetKeyboardFocusHere();
            focusFirstField = false;
        }
    }

    // ---------- Screens ----------
    void welcome(Gui::AppState& app, float width) {
        const Theme::Palette& p = Theme::colors();
        Widgets::badge("CS160  ·  GROUP 4", p.blue);
        ImGui::Dummy(ImVec2(0, px(4)));
        title("Personal Finance Manager", 34.0f);
        subtitle("Track your income, expenses, wallets and monthly payments, all in one place.");
        ImGui::Dummy(ImVec2(0, px(18)));
        if (Widgets::button(ICON_FA_RIGHT_TO_BRACKET "  Log in", p.blue, ImVec2(width, px(44)))) {
            goTo(app, Gui::AuthScreen::LOGIN);
        }
        if (Widgets::ghostButton(ICON_FA_USER_PLUS "  Create account", ImVec2(width, px(44)))) {
            goTo(app, Gui::AuthScreen::REGISTER);
        }
    }

    void loginForm(Gui::AppState& app, float width) {
        const Theme::Palette& p = Theme::colors();
        backButton(app);
        title("Welcome back", Theme::FONT_HUGE);
        subtitle("Log in to see your wallets and transactions.");
        ImGui::Dummy(ImVec2(0, px(8)));

        Widgets::fieldLabel("USERNAME");
        focusHere();
        Widgets::textInput("##loginUser", loginUser, "Your username");
        Widgets::fieldLabel("PASSWORD");
        bool enter = Widgets::passwordInput("##loginPassword", loginPassword, "Your password");
        Widgets::formError(loginError);
        ImGui::Dummy(ImVec2(0, px(6)));

        if (Widgets::button(ICON_FA_RIGHT_TO_BRACKET "  Log in", p.blue, ImVec2(width, px(44))) || enter) {
            int index = Auth::login(app.users, loginUser, loginPassword, loginError);
            if (index != -1) {
                loginPassword = "";
                Gui::login(app, index);
                return;
            }
        }
        ImGui::Dummy(ImVec2(0, px(4)));
        footerLink("New here?", "Create an account", app, Gui::AuthScreen::REGISTER);
    }

    void registerForm(Gui::AppState& app, float width) {
        const Theme::Palette& p = Theme::colors();
        backButton(app);
        title("Create your account", Theme::FONT_HUGE);
        subtitle("Your data is kept separately for each account.");
        ImGui::Dummy(ImVec2(0, px(8)));

        Widgets::fieldLabel("NICKNAME");
        focusHere();
        Widgets::textInput("##regNickname", regNickname, "Shown as \"Hello, ...\"");
        Widgets::fieldLabel("USERNAME");
        Widgets::textInput("##regUser", regUser, "3-20 letters, digits, _ or .");
        Widgets::fieldLabel("PASSWORD");
        bool enter = Widgets::passwordInput("##regPassword", regPassword, "At least 6 characters");
        Widgets::fieldLabel("CONFIRM PASSWORD");
        enter = Widgets::passwordInput("##regConfirm", regConfirm, "Type the password again") || enter;
        Widgets::formError(regError);
        ImGui::Dummy(ImVec2(0, px(6)));

        if (Widgets::button(ICON_FA_USER_PLUS "  Create account", p.blue, ImVec2(width, px(44))) || enter) {
            if (Auth::registerUser(app.users, regUser, regNickname, regPassword, regConfirm, regError)) {
                if (!Auth::saveUsers(app.users, app.usersFile)) {
                    app.users.removeAt(app.users.size() - 1);
                    regError = "Could not save the account to " + app.usersFile + ".";
                } else {
                    int index = app.users.size() - 1;
                    regNickname = regUser = regPassword = regConfirm = "";
                    Gui::login(app, index);
                    return;
                }
            }
        }
        ImGui::Dummy(ImVec2(0, px(4)));
        footerLink("Already have an account?", "Log in", app, Gui::AuthScreen::LOGIN);
    }
}

namespace Pages {

void auth(Gui::AppState& app) {
    loadImages();
    const Theme::Palette& p = Theme::colors();
    ImVec2 origin = ImGui::GetCursorScreenPos();
    ImVec2 size = ImGui::GetContentRegionAvail();
    float split = std::floor(size.x * 0.52f);

    heroPanel(origin, ImVec2(origin.x + split, origin.y + size.y));
    ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(origin.x + split, origin.y),
                                              ImVec2(origin.x + size.x, origin.y + size.y), Theme::toU32(p.card));

    // Form column, centered in the right half (height measured on the previous frame)
    int screen = (int)app.authScreen;
    float formWidth = std::fmin(px(400), (size.x - split) - px(64));
    float x = origin.x + split + ((size.x - split) - formWidth) * 0.5f;
    float y = origin.y + std::fmax(px(24), (size.y - formHeight[screen]) * 0.5f);
    ImGui::SetCursorScreenPos(ImVec2(x, y));

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(px(8), px(8)));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(px(12), px(10)));
    ImGui::BeginChild("##authForm", ImVec2(formWidth, 0), ImGuiChildFlags_AutoResizeY,
                      ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar);
    switch (app.authScreen) {
        case Gui::AuthScreen::WELCOME:  welcome(app, formWidth); break;
        case Gui::AuthScreen::LOGIN:    loginForm(app, formWidth); break;
        case Gui::AuthScreen::REGISTER: registerForm(app, formWidth); break;
    }
    formHeight[screen] = ImGui::GetCursorPosY();
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
}

}
