// editor.cpp - the character editor (F12 > Editor, switched on under Helpers): what the
// save-game editors of the community (Darklands Companion, EDITOR.EXE, CharacterEditor) change,
// done on the running game: money, names, attributes, skills, saints, formulas, a character's
// colours (the "garish colors" bug), shield and figure, items' quality and number.
// Party: fame DS:9082, bank notes DS:9072, philosopher's stone quality DS:9078 (the save header's
// 0x7a, 0x8c, 0x92), local reputation location +0x12.
// Member record DS:9c55 + 0x80*m (the save's character record 0x00..0x7f: +0x12 age, +0x15
// shield letter, +0x25 name, +0x3e nickname, +0x5d/+0x64 attributes, +0x6b skills, +0x7e items);
// colours far 20a5:33fc [5][24] (8 RGB, 6 bits); figure DS:a668 [5][4] ("F60"); saints far
// 2e38:082c [5][20] (bit per saint, highest first); formulas far 2e38:07be [5][22] (bits 0-2:
// the three qualities of a potion's formula).
#include <stdio.h>
#include <string.h>
#include <string>
#include "imgui.h"
#include "ui.h"
#include "gamedata.h"
#include "skillicons.h"
#include "../mods/looks.h"
#include "uiinternal.h"
extern "C" {
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "../mods/gamemodel.h"
}

static const char *FIGURES[] = { "F01", "F60", "C00", "A00" };      /* the game's own; then the enemies' (mods/looks.c) */

static u16 rec(int m) { return (u16)(0x9c55 + 0x80 * m); }
#define G_FAR20A5 ((u16)(LOADSEG + 0x20a5))

static void byte_slider(const char *label, u16 seg, u16 off, int lo, int hi)
{
    int v = rb(seg, off);
    ImGui::SetNextItemWidth(UIS(160));
    if (ImGui::SliderInt(label, &v, lo, hi)) wb(seg, off, (u8)v);
}

static void name_edit(const char *label, u16 off, int n)
{
    char b[32]; g_str(G_DGROUP, off, b, n);
    ImGui::SetNextItemWidth(UIS(220));
    if (ImGui::InputText(label, b, (size_t)n)) for (int i = 0; i < n; i++) g_wb((u16)(off + i), (u8)(i < (int)strlen(b) ? b[i] : 0));
}

extern "C" void tab_editor(void)
{
    if (game_mode() == GM_OTHER) { ImGui::TextWrapped("Start or load a game first."); return; }
    ImGui::TextColored(ImVec4(0.55f, 0.2f, 0.1f, 1), "Changes here apply to the running game at once. Save afterwards to keep them.");
    {
        int f = (s16)g_rw(G_MONEY), g = (s16)g_rw(G_MONEY + 2), p = (s16)g_rw(G_MONEY + 4);
        ImGui::SetNextItemWidth(UIS(110)); if (ImGui::InputInt("florins", &f)) g_ww(G_MONEY, (u16)(f < 0 ? 0 : f));
        ImGui::SameLine(); ImGui::SetNextItemWidth(UIS(110)); if (ImGui::InputInt("groschen", &g)) g_ww(G_MONEY + 2, (u16)(g < 0 ? 0 : g));
        ImGui::SameLine(); ImGui::SetNextItemWidth(UIS(110)); if (ImGui::InputInt("pfennigs", &p)) g_ww(G_MONEY + 4, (u16)(p < 0 ? 0 : p));
        int bn = (s16)g_rw(GV_party_notes), fame = (s16)g_rw(GV_party_fame), ps = (s16)g_rw(GV_philosophers_stone);
        ImGui::SetNextItemWidth(UIS(110)); if (ImGui::InputInt("florins in bank notes", &bn)) g_ww(GV_party_notes, (u16)(bn < 0 ? 0 : bn));
        ImGui::SameLine(); ImGui::SetNextItemWidth(UIS(110)); if (ImGui::InputInt("fame", &fame)) g_ww(GV_party_fame, (u16)fame);
        ImGui::SameLine(); ImGui::SetNextItemWidth(UIS(110)); if (ImGui::InputInt("Philosopher's Stone quality", &ps)) g_ww(GV_philosophers_stone, (u16)(ps < 0 ? 0 : ps > 99 ? 99 : ps));
        int loc = (s16)g_rw(G_CUR_LOC);
        u32 lr = loc >= 0 && loc < G_NLOC ? g_location(loc) : 0;
        if (lr) {
            u16 ls = (u16)(lr >> 16), lo = (u16)lr;
            char ln[24]; g_str(ls, (u16)(lo + 0x26), ln, sizeof ln);
            int rep = (s16)rw(ls, (u16)(lo + 0x12));
            std::string lab = "reputation in " + game_text(ln, sizeof ln);
            ImGui::SetNextItemWidth(UIS(110)); if (ImGui::InputInt(lab.c_str(), &rep)) ww(ls, (u16)(lo + 0x12), (u16)rep);
        }
    }
    int size = (s16)g_rw(G_PARTY_SIZE);
    if (size < 1 || size > 5) return;
    if (!ui_subtabs_begin("edmembers")) return;
    for (int m = 0; m < size; m++) {
        char nick[16]; g_str(G_DGROUP, (u16)(rec(m) + 0x3e), nick, sizeof nick);
        std::string tabname = game_text(nick, sizeof nick) + "##ed" + std::to_string(m);
        if (!ui_subtab(tabname.c_str())) continue;
        ImGui::BeginChild("edm", ImVec2(0, 0));
        u16 r = rec(m);
        if (ImGui::CollapsingHeader("Name, age, crest and sprite", ImGuiTreeNodeFlags_DefaultOpen)) {
            name_edit("full name", (u16)(r + 0x25), 25);
            name_edit("nickname", (u16)(r + 0x3e), 11);
            int age = (s16)g_rw((u16)(r + 0x12));
            ImGui::SetNextItemWidth(UIS(110)); if (ImGui::InputInt("age", &age)) g_ww((u16)(r + 0x12), (u16)age);
            int sh = g_rb((u16)(r + 0x15)) - 'A';
            ImGui::SetNextItemWidth(UIS(110));
            if (ImGui::SliderInt("crest (A..O)", &sh, 0, 14, "")) g_wb((u16)(r + 0x15), (u8)('A' + sh));
            ImGui::SameLine(); ImGui::Text("%c", 'A' + sh);
            char fig[4]; g_str(G_DGROUP, (u16)(0xa668 + 4 * m), fig, sizeof fig);
            ImGui::SetNextItemWidth(UIS(110));
            std::string cur = std::string(fig) + "  " + looks_title_of(fig);
            if (ImGui::BeginCombo("sprite", cur.c_str())) {
                auto item = [&](const char *f) {
                    std::string lab = std::string(f) + "  " + looks_title_of(f);
                    if (ImGui::Selectable(lab.c_str(), !strncmp(f, fig, 3))) looks_set_figure(m, f);
                };
                for (const char *f : FIGURES) item(f);
                for (int i = 0; i < looks_count(); i++) if (looks_usable(i)) item(looks_code(i));
                ImGui::EndCombo();
            }
            if (looks_is_extra(fig)) { ImGui::SameLine(); ImGui::TextDisabled("(an enemy's sprite: its colors below are its own shades)"); }
            ImGui::TextDisabled("Colors of the sprite and its portrait (battles and the party panel):");
            for (int c = 0; c < 8; c++) {
                u16 o = (u16)(0x33fc + m * 24 + c * 3);
                float col[3] = { (rb(G_FAR20A5, o) & 63) / 63.0f, (rb(G_FAR20A5, (u16)(o + 1)) & 63) / 63.0f, (rb(G_FAR20A5, (u16)(o + 2)) & 63) / 63.0f };
                ImGui::PushID(c);
                if (ImGui::ColorEdit3("##c", col, ImGuiColorEditFlags_NoInputs)) for (int k = 0; k < 3; k++) wb(G_FAR20A5, (u16)(o + k), (u8)(col[k] * 63 + 0.5f));
                ImGui::PopID();
                if (c != 7) ImGui::SameLine();
            }
        }
        if (ImGui::CollapsingHeader("Attributes (current / maximum)", ImGuiTreeNodeFlags_DefaultOpen)) {
            for (int a = 0; a < 7; a++) {
                ImGui::PushID(a);
                byte_slider("##cur", G_DGROUP, (u16)(r + 0x5d + a), 0, 99); ImGui::SameLine();
                byte_slider(gm_attr_name(a), G_DGROUP, (u16)(r + 0x64 + a), 1, 99);
                ImGui::PopID();
            }
        }
        if (ImGui::CollapsingHeader("Skills")) {
            for (int k = 0; k < 19; k++) { ImGui::PushID(100 + k); ui_skill_icon_inline(k); byte_slider(gm_skill_name(k), G_DGROUP, (u16)(r + 0x6b + k), 0, 99); ImGui::PopID(); }
        }
        if (ImGui::CollapsingHeader("Saints known")) {
            for (int s = 0; s < (int)gamedata.saints.size(); s++) {
                u16 o = (u16)(0x082c + m * 0x14 + (s >> 3)); u8 bit = (u8)(0x80 >> (s & 7));
                bool k = (rb(G_FAR2E38, o) & bit) != 0;
                ImGui::PushID(200 + s);
                if (ImGui::Checkbox(gamedata.saints[s].name.c_str(), &k)) wb(G_FAR2E38, o, (u8)(k ? rb(G_FAR2E38, o) | bit : rb(G_FAR2E38, o) & ~bit));
                ImGui::PopID();
                if (s % 3 != 2) ImGui::SameLine(ImGui::GetContentRegionAvail().x / 3 * (float)((s % 3) + 1) + 8);
            }
            ImGui::NewLine();
        }
        if (ImGui::CollapsingHeader("Formulae known (each potion has three)")) {
            for (int p = 0; p < 22; p++) {
                u16 o = (u16)(0x07be + m * 0x16 + p);
                const char *pn = 95 + p < (int)gamedata.items.size() ? gamedata.items[95 + p].name.c_str() : "?";
                ImGui::PushID(400 + p);
                ImGui::TextUnformatted(pn);
                for (int q = 0; q < 3; q++) {
                    u8 bit = (u8)(1 << q);
                    bool k = (rb(G_FAR2E38, o) & bit) != 0;
                    ImGui::SameLine(UIS(260 + 70.0f * q));
                    char lb[16]; snprintf(lb, sizeof lb, "%d##%d", q + 1, q);
                    if (ImGui::Checkbox(lb, &k)) wb(G_FAR2E38, o, (u8)(k ? rb(G_FAR2E38, o) | bit : rb(G_FAR2E38, o) & ~bit));
                }
                ImGui::PopID();
            }
        }
        if (ImGui::CollapsingHeader("Items: quality and count")) {
            int n = g_rb((u16)(G_MEMBER(m) + G_M_NITEMS));
            for (int i = 0; i < n && i < 64; i++) {
                u16 o = G_ITEM(m, i);
                int code = rw(G_FAR2E38, o) & 0xfff;
                std::string nm = code < (int)gamedata.items.size() ? game_text(gamedata.items[code].name.c_str(), 64) : "#" + std::to_string(code);
                ImGui::PushID(600 + i);
                int q = rb(G_FAR2E38, (u16)(o + 3)), c = rb(G_FAR2E38, (u16)(o + 4));
                ImGui::SetNextItemWidth(UIS(130)); if (ImGui::SliderInt("##q", &q, 1, 99, "quality %d")) wb(G_FAR2E38, (u16)(o + 3), (u8)q);
                ImGui::SameLine(); ImGui::SetNextItemWidth(UIS(110)); if (ImGui::InputInt("##n", &c)) wb(G_FAR2E38, (u16)(o + 4), (u8)(c < 1 ? 1 : c > 99 ? 99 : c));
                ImGui::SameLine(); ImGui::TextUnformatted(nm.c_str());
                ImGui::PopID();
            }
        }
        ImGui::EndChild();
        ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
}
