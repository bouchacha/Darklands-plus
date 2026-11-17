/* teacherfinder.cpp - the world map's lessons, looked up for one member and skill. */
#include "teacherfinder.h"
#include "teachersui.h"
#include "gamedata.h"
#include "libraries.h"
#include "money.h"
#include "richtext.h"
#include "skillicons.h"
#include "ui.h"
#include "../mods/fixes.h"
#include "../mods/gamemodel.h"
#include "../runtime/game.h"
#include <algorithm>
#include <climits>
#include <cfloat>

extern ImFont *ui_font_bold;

namespace {
struct Lesson {
    std::string where, details;
    int skill, ability, fee;
    bool refused;
};
struct Town {
    int loc, minutes;
    std::string name;
    std::vector<Lesson> lessons;
};
std::vector<Town> towns;
struct Match { const Town *town; const Lesson *lesson; int limit; };
std::vector<Match> matches;
std::string no_lessons;
int student = -1, subject = SKILL_ALCHEMY;
bool refresh, combo_open, close_combo;

std::string member_name(int slot)
{
    char name[32];
    gm_member_name(slot, name, sizeof name);
    return game_text(name, sizeof name);
}

void read_towns(const std::vector<Place> &places)
{
    towns.clear();
    for (const Place &p : places) {
        if (p.loc < 0 || p.loc >= (int)gamedata.cities.size() || !place_offers_known(p.loc)) continue;
        Town town = {p.loc, worldmap_land_minutes(p.loc), p.name, {}};
        TchHired hired[G_NEVENTS];
        int nh = teach_hired(p.loc, hired, G_NEVENTS);
        for (int i = 0; i < nh; i++) {
            const auto &h = hired[i];
            if (h.skill < 0 || h.skill >= 19 || teach_best_at(p.loc, h.skill) != h.event) continue;
            std::string until = std::to_string(h.day) + " " + gm_month_name(h.month % 12);
            town.lessons.push_back({"Inn (until " + until + ")", "Hired until " + until + ". Choose \"Train or study\" at the inn while the teacher is there.",
                                    h.skill, h.strength, h.fee, false});
        }
        TchSource sources[12];
        int n = ui_town_teachers(p.loc, sources, 12);
        for (int i = 0; i < n; i++) {
            const auto &s = sources[i];
            std::string details = ui_teacher_text(s);
            for (int k = 0; k < s.n; k++)
                town.lessons.push_back({s.who, details, s.skill[k], s.strength, s.fee[k], s.off != 0});
        }
        towns.push_back(std::move(town));
    }
    std::stable_sort(towns.begin(), towns.end(), [](const Town &a, const Town &b) {
        int da = a.minutes < 0 ? INT_MAX : a.minutes, db = b.minutes < 0 ? INT_MAX : b.minutes;
        return da == db ? a.name < b.name : da < db;
    });
}

void filter_lessons()
{
    matches.clear();
    const int value = gm_member_skill(student, subject);
    const bool capped = fix_is_on("teachcap");
    int highest = -1;
    bool refused = false;
    for (const Town &town : towns) for (const Lesson &lesson : town.lessons) {
        if (lesson.skill != subject) continue;
        int limit = capped ? std::min(99, lesson.ability) : 99;
        if (value >= limit) highest = std::max(highest, limit);
        else if (lesson.refused) refused = true;
        else matches.push_back({&town, &lesson, limit});
    }
    if (!matches.empty()) return;
    std::string name = member_name(student), skill = gm_skill_name(subject);
    if (value >= 99) no_lessons = name + " has reached 99 in " + skill + ".";
    else if (refused) no_lessons = "No known teacher can take " + name + " for " + skill + " lessons at present.";
    else if (highest >= 0) no_lessons = name + " already has " + std::to_string(value) + " in " + skill +
        ". The best teacher the party knows teaches up to " + std::to_string(highest) + ".";
    else no_lessons = "The party knows no teacher of " + skill + " yet. Visit more towns to learn who teaches there.";
}

bool choose_filters()
{
    bool changed = false;
    if (!ImGui::BeginTable("##teacher_filters", 2, ImGuiTableFlags_SizingStretchSame)) return false;
    ImGui::TableNextColumn();
    ImGui::TextUnformatted("Student");
    ImGui::TableNextColumn();
    ui_skill_icon_inline(subject); ImGui::TextUnformatted("Skill");
    ImGui::TableNextColumn();
    ImGui::SetNextItemWidth(-1);
    if (ImGui::BeginCombo("##student", member_name(student).c_str())) {
        combo_open = true;
        for (int m = 0; m < gm_party_size(); m++) {
            if (!gm_member_present(m) || gm_member_status(m) == 5) continue;
            ImGui::PushID(m);
            if (ImGui::Selectable(member_name(m).c_str(), m == student)) { student = m; changed = true; }
            if (m == student) ImGui::SetItemDefaultFocus();
            ImGui::PopID();
        }
        if (close_combo) ImGui::CloseCurrentPopup();
        ImGui::EndCombo();
    }
    ImGui::TableNextColumn();
    auto skill_label = [](int k) {
        return std::string(gm_skill_name(k)) + " (" + std::to_string(gm_member_skill(student, k)) + ")";
    };
    ImGui::SetNextItemWidth(-1);
    if (ImGui::BeginCombo("##skill", skill_label(subject).c_str())) {
        combo_open = true;
        for (int k = 0; k < 19; k++) {
            if (!teach_skill_offered(k)) continue;
            if (ImGui::Selectable(skill_label(k).c_str(), k == subject)) { subject = k; changed = true; }
            if (k == subject) ImGui::SetItemDefaultFocus();
        }
        if (close_combo) ImGui::CloseCurrentPopup();
        ImGui::EndCombo();
    }
    ImGui::EndTable();
    return changed;
}

/* The whole row selects the town. The location, limit and fee stay visible without a tooltip. */
bool lesson_row(const Town &town, const Lesson &lesson, int limit)
{
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    const ImVec2 start = ImGui::GetCursorPos();
    const float height = ImGui::GetTextLineHeight() * 2 + ImGui::GetStyle().ItemSpacing.y;
    bool picked = ImGui::Selectable("##lesson", false, ImGuiSelectableFlags_SpanAllColumns, ImVec2(0, height));
    bool hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal);
    ImGui::SetCursorPos(start);
    if (ui_font_bold) ImGui::PushFont(ui_font_bold);
    ImGui::TextUnformatted(town.name.c_str());
    if (ui_font_bold) ImGui::PopFont();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetStyle().ItemSpacing.y * 0.5f);
    ImGui::TextWrapped("%s", lesson.where.c_str());
    ImGui::TableNextColumn();
    ImGui::TextWrapped("%s", town.minutes < 0 ? "No land route" : town.minutes == 0 ? "Here" : hhmm(town.minutes).c_str());
    ImGui::TableNextColumn();
    ImGui::Text("%d", limit);
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(money_str(lesson.fee).c_str());
    if (hovered) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + UIS(355));
        ui_rich_text(lesson.details.c_str());
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
    return picked;
}
}

bool teacher_finder_close_combo()
{
    close_combo = combo_open;
    return close_combo;
}

void teacher_finder_start(int member, int skill)
{
    static int seed = -1;
    if (seed != g_rw(G_PARTY_SEED)) {
        seed = g_rw(G_PARTY_SEED); student = gm_leader(); subject = SKILL_ALCHEMY;
    }
    if (member >= 0) student = member;
    if (!gm_member_present(student) || gm_member_status(student) == 5) student = gm_leader();
    if (skill >= 0 && skill < 19) subject = skill;
    refresh = true;
    combo_open = close_combo = false;
}

int teacher_finder_window(bool &open, const std::vector<Place> &places, float scale, float pad)
{
    bool reset_scroll = refresh;
    if (refresh) { read_towns(places); filter_lessons(); refresh = false; }
    const ImVec2 screen = ImGui::GetIO().DisplaySize;
    float width = std::min(screen.x - 2 * pad, UIS(530) * scale);
    float max_h = std::min(screen.y - 2 * pad, UIS(490) * scale);
    ImGui::SetNextWindowPos(ImVec2(screen.x * 0.5f, screen.y * 0.16f), ImGuiCond_Appearing, ImVec2(0.5f, 0));
    ImGui::SetNextWindowSizeConstraints(ImVec2(width, 0), ImVec2(width, max_h));
    ImGui::SetNextWindowBgAlpha(0.98f);
    int picked = -1;
    if (ImGui::Begin("Find a teacher###teachers", &open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize |
                     ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar)) {
        ImGui::SetWindowFontScale(scale);
        combo_open = false;
        if (choose_filters()) { filter_lessons(); reset_scroll = true; }
        close_combo = false;
        ImGui::Spacing();
        if (!matches.empty()) {
            ImGui::TextWrapped("Nearest first by land. Point at a teacher for hiring terms. Click a row to show the town on the map.");
            ImGui::Spacing();
        }
        const float room = std::max(1.0f, max_h - ImGui::GetCursorPosY() - ImGui::GetStyle().WindowPadding.y);
        ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(FLT_MAX, room));
        if (ImGui::BeginChild("##lessons", ImVec2(0, 0), ImGuiChildFlags_AutoResizeY)) {
            if (reset_scroll) ImGui::SetScrollY(0);
            if (matches.empty()) ImGui::TextWrapped("%s", no_lessons.c_str());
            else if (ImGui::BeginTable("##offers", 4, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_RowBg)) {
                ImGui::TableSetupColumn("Town and teacher", ImGuiTableColumnFlags_WidthStretch, 2.4f);
                ImGui::TableSetupColumn("By land", ImGuiTableColumnFlags_WidthStretch, 1.0f);
                ImGui::TableSetupColumn("Up to", ImGuiTableColumnFlags_WidthStretch, 0.65f);
                ImGui::TableSetupColumn("Daily fee", ImGuiTableColumnFlags_WidthStretch, 1.0f);
                ImGui::TableHeadersRow();
                for (const Match &match : matches) {
                    ImGui::PushID(match.lesson);
                    if (lesson_row(*match.town, *match.lesson, match.limit)) { picked = match.town->loc; open = false; }
                    ImGui::PopID();
                }
                ImGui::EndTable();
            }
        }
        ImGui::EndChild();
    }
    ImGui::End();
    return picked;
}
