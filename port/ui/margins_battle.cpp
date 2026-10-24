/* margins_battle.cpp - the margins in a battle: the right margin's fighters, their orders as
 * buttons (with the potions' pictures), the enemies still waiting; the order buttons at the foot of
 * the field when the window has no margin. Split from ui/margins.cpp (margins_parts.h). */
#include <stdio.h>
#include <float.h>
#include <string.h>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include "padglyphs.h"
#include "imgui.h"
#include "gamedata.h"
#include "teachersui.h"
#include "ordericons.h"
#include "ui.h"
extern "C" {
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "../mods/travel.h"
#include "../mods/menuview.h"
#include "../mods/assist.h"
#include "../mods/foesleft.h"
#include "../mods/gamemodel.h"
#include "../mods/fixes.h"
#include "../mods/saintinfo.h"
void plat_game_to_win(float gx, float gy, float *wx, float *wy);
}
extern ImFont *ui_font_text;
extern ImFont *ui_font_overlay;          /* ui.cpp: the text over and beside the picture */
#include "effects.h"
#include "quests.h"
#include "townicons.h"
#include "libraries.h"
#include <functional>
#include <set>
#include "money.h"
#include "margins_parts.h"
extern "C" int encounters_card_active(void);
extern "C" int encounters_builtin(int i);
extern "C" void ui_open_goto(void);
extern "C" int citynav_available(void);
extern "C" int battlelog_margin(float x, float w, float h, float fs);
extern "C" int battle_showing(void);                  /* ui/battlelog.cpp: a battle is on (not the marching order) */

extern int ui_margins_on;

extern "C" int levelmap_available(void);

extern "C" int politics_unrest_at(int loc);           /* (mods/politics.c) */
extern "C" int war_state(int loc, int *left);          /* (mods/war.c) */
struct OrderBtn { ImVec2 a, b; int m, i, s; };
static std::vector<OrderBtn> order_btns; static int order_btns_frame = -100;
int margins_order_click(int x, int y)
{
    if (ImGui::GetFrameCount() - order_btns_frame > 2) return 0;
    for (const OrderBtn &o : order_btns)
        if (x >= o.a.x && x < o.b.x && y >= o.a.y && y < o.b.y) { menuview_request(o.m, o.i, o.s); return 1; }
    return 0;
}
/* a potion among the orders (the game's short name; its quality is the entry's "key", "q30"),
   for its picture on the button; -1 none */
void ui_item_icon_q(ImDrawList *dl, ImVec2 c, float s, int id, int q);
static int potion_of(const std::string &name, const char *key, int *q)
{
    for (int id = 95; id <= 116 && id < (int)gamedata.items.size(); id++)
        if (!gamedata.items[id].shortname.empty() && name == gamedata.items[id].shortname) {
            *q = key[0] == 'q' ? atoi(key + 1) : -1;
            return id;
        }
    return -1;
}
extern int ui_battle_info;                       /* ui/battleoverlay.cpp: battle.info */
/* what each order does, under the pointer (s016 1ef8's handlers, s014 0e76 and 18bf: what an order does in a fight) */
static const char *order_tip(int kind)
{
    switch (kind) {
    case OI_RESUME: return "The battle runs again (the key: Space). Any order being given is given first.";
    case OI_DONE: return "This member's order is given; the battle stays paused, to give others theirs (the key: Enter).";
    case OI_INFO: return "Every kind of enemy on the field: attributes, weapons, armor (the key: E).";
    case OI_WALK: return "Then click a spot: the member walks there.";
    case OI_FLEE: return "Then click a spot: the member runs there without stopping to fight or striking back (to get out of a melee).";
    case OI_HALT: return "The member stops where they are and waits.";
    case OI_GROUP: return "The party moves together as a group.";
    case OI_FILE: return "The party moves in single file, the leader first.";
    case OI_DOOR: return "Open or shut the door beside the member.";
    case OI_STAIRS: return "Take the stairs beside the member to another floor.";
    case OI_CHEST: return "Open the chest beside the member.";
    case OI_PICK: return "Pick the lock beside the member.";
    case OI_DISSOLVE: return "Dissolve the lock beside the member with an Eater-Water potion.";
    case OI_DISARM: return "Disarm the trap on the chest beside the member.";
    case OI_SURRENDER: return "The whole party gives up the fight.";
    case OI_LOOT: return "Search the fallen for their belongings.";
    case OI_EXIT: return "Leave the battlefield by the way out beside the member.";
    case OI_CANCEL: return "Stop giving this member an order (the key: Esc); the battle stays paused.";
    case OI_ATTACK: return "Then click an enemy: a standard attack, balanced between blows and guard.";
    case OI_VULNERABLE: return "Then click an enemy: blows aimed at the gaps in the armor (1 to 4 more penetration), but slower and a little less likely to land.";
    case OI_BERSERK: return "Then click an enemy: quick, furious blows without thought of defence.";
    case OI_PARRY: return "Then click an enemy: the member mostly defends, hard to hit, striking back only now and then.";
    case OI_MISSILE: return "Then click an enemy: shoot or throw at it with the missile weapon (a clear line needed).";
    }
    return nullptr;
}
static void order_buttons(float width)
{
    order_btns.clear();
    if (!menuview_is_on() || g_rb(GV_order_mode) != 1) return;
    int sel = (s8)g_rb(GV_selected_member);
    if (sel < 0 || sel > 4) return;
    const MenuModel *M = menuview_live();
    char nm[32]; { u16 b = G_MEMBER(sel); int i = 0; for (; i < 31; i++) { char c = (char)g_rb((u16)(b + 0x2a + i)); if (!c) break; nm[i] = c; } nm[i] = 0; }
    std::string who = game_text(nm, strlen(nm)); size_t sp = who.find(' '); if (sp != std::string::npos) who = who.substr(0, sp);
    bool head = false;
    ImDrawList *dl = ImGui::GetWindowDrawList();
    ImVec2 o0 = ImGui::GetCursorScreenPos(), mp = ImGui::GetIO().MousePos;
    float x0 = o0.x, x1 = o0.x + width - ImGui::GetStyle().WindowPadding.x * 2, x = x0, y = 0, lh = ImGui::GetTextLineHeight(), pad = lh * 0.3f, gap = lh * 0.25f;
    for (int j = 0; j < M->nmenus; j++) {
        std::string mn = M->menu[j].name;
        if (!M->menu[j].enabled || (mn != "Attack" && mn != "Orders")) continue;
        for (int k = 0; k < M->menu[j].n; k++) {
            int ix = M->menu[j].first + k;
            const MvItem &it = M->items[ix];
            int nsub = M->sub_n[ix];
            for (int s2 = nsub ? 0 : -1; s2 < (nsub ? nsub : 0); s2++) {
                const MvItem &e = s2 >= 0 ? M->items[M->sub_first[ix] + s2] : it;
                if (!e.enabled || (s2 >= 0 && !it.enabled)) continue;
                if (!head) { heading(("Orders for " + who).c_str()); head = true; y = ImGui::GetCursorScreenPos().y; x = x0; }
                std::string label = game_text(e.name, strlen(e.name));
                if (s2 >= 0) label = game_text(it.name, strlen(it.name)) + ": " + (e.count > 1 ? std::to_string(e.count) + " " : "") + label;
                int pq = -1, pid = s2 >= 0 ? potion_of(game_text(e.name, strlen(e.name)), e.key, &pq) : -1;
                int kind = s2 < 0 ? ui_order_icon_for(game_text(e.name, strlen(e.name)).c_str()) : OI_NONE;
                /* (not what is at hand otherwise: Enemy Info when pointing at an enemy tells it, and
                   "<name> finished" when a click gives the order) */
                if ((kind == OI_INFO && ui_battle_info) || (kind == OI_DONE && config_get_int("battle.quick_orders", 1))) continue;
                int oi = pid < 0 && config_get_int("battle.order_icons", 1) ? kind : OI_NONE;
                if (e.key[0] && pid < 0) label += std::string(" (") + e.key + ")";      /* (a potion's quality: the dots on its picture) */
                float iw = pid >= 0 || oi != OI_NONE ? lh * 1.15f : 0;   /* (a potion: its picture, with its grade; an order: ui/ordericons.cpp) */
                ImVec2 ts = ImGui::CalcTextSize(label.c_str());
                float w = ts.x + pad * 2 + iw, h = lh + pad;
                if (x > x0 && x + w > x1) { x = x0; y += h + gap; }
                ImVec2 a(x, y), b(x + w, y + h);
                bool hov = mp.x >= a.x && mp.x < b.x && mp.y >= a.y && mp.y < b.y;
                dl->AddRectFilled(a, b, hov ? IM_COL32(150, 112, 56, 255) : IM_COL32(96, 70, 36, 255), h * 0.2f);
                dl->AddRect(a, b, IM_COL32(170, 132, 70, 255), h * 0.2f);
                if (pid >= 0) ui_item_icon_q(dl, ImVec2(a.x + pad + iw * 0.45f, a.y + h * 0.5f), iw, pid, pq);
                else if (oi != OI_NONE) ui_draw_order_icon(dl, ImVec2(a.x + pad + iw * 0.42f, a.y + h * 0.5f), iw * 0.95f, oi);
                dl->AddText(ImVec2(a.x + pad + iw, a.y + pad * 0.5f), IM_COL32(244, 232, 204, 255), label.c_str());
                order_btns.push_back({ a, b, j, k, s2 });
                if (hov && order_tip(kind)) margin_tip(order_tip(kind));
                x += w + gap;
            }
        }
    }
    if (head) {
        ImGui::SetCursorScreenPos(ImVec2(x0, y + lh + pad + gap));
        ImGui::Dummy(ImVec2(1, 1));
        ImGui::Separator();
        order_btns_frame = ImGui::GetFrameCount();
    }
}

/* the foes the place still holds, not yet met (mods/foesleft.c): where the rest of a castle's or
   dungeon's defenders wait, so a fight that "does not end" says why */
static void foes_left_section(void)
{
    FoesLeft fl;
    if (!foesleft_on() || !foesleft_get(&fl)) return;
    int here = fl.foes[fl.floor], others = 0;
    for (int f = 0; f < fl.floors; f++) if (f != fl.floor && fl.known[f]) others += fl.foes[f];
    if (fl.floors <= 1 && !here) return;                     /* (an open field with no one waiting: nothing to say) */
    ImGui::Spacing();
    heading("Enemies not yet met");
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    if (!here && !others) ImGui::TextWrapped("No one else waits in this place.");
    else {
        if (here && fl.floors <= 1) ImGui::TextWrapped("Here: %d more %s, in %d %s.", here, here == 1 ? "enemy" : "enemies", fl.groups[fl.floor], fl.groups[fl.floor] == 1 ? "group" : "groups");
        else if (here) ImGui::TextWrapped("On this floor (%d): %d %s, in %d %s.", fl.floor + 1, here, here == 1 ? "enemy" : "enemies", fl.groups[fl.floor], fl.groups[fl.floor] == 1 ? "group" : "groups");
        else ImGui::TextWrapped("No one on this floor (%d).", fl.floor + 1);
        std::string o;
        for (int f = 0; f < fl.floors; f++) {
            if (f == fl.floor || !fl.known[f] || !fl.foes[f]) continue;
            char b[48]; snprintf(b, sizeof b, "%sfloor %d: %d", o.empty() ? "" : ", ", f + 1, fl.foes[f]);
            o += b;
        }
        if (!o.empty()) ImGui::TextWrapped("Elsewhere: %s.", o.c_str());
        if (fl.master_floor >= 0) {
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(214, 120, 90, 255));
            if (fl.master_floor == fl.floor) ImGui::TextWrapped("The master of this place waits on this floor.");
            else ImGui::TextWrapped("The master of this place waits on floor %d.", fl.master_floor + 1);
            ImGui::PopStyleColor();
        }
    }
    ImGui::PopStyleColor();
}

void battle_panel(float width, float fs)
{
    static bool seen[13]; static u16 last_tick = 0xffff;
    static int last_st[13]; static std::string last_name[13];      /* the fallen: the game clears their records */
    u16 tick = g_rw(GV_battle_tick);                        /* the battle's clock: back at 0 in a new battle */
    if (tick < last_tick) for (int i = 0; i < 13; i++) { seen[i] = false; last_st[i] = 0; last_name[i].clear(); }
    last_tick = tick;
    int n = (s8)g_rb(GV_hotspot_count);
    for (int k = 0; k < n && k < 40; k++) { int m = g_rb((u16)(GV_hotspot_cbt + k)); if (m < 13) seen[m] = true; }
    float bw = width - fs * 1.6f, bh = fs * 0.32f;
    if (config_get_int("battle.fast_key", 1) && !config_get_int("original", 0) && game_mode() == GM_BATTLE) {   /* fast forward (Z, mods/pathfix.c): on the field (not over a sheet or the loot) */
        const bool padp = ui_pad_prompts();               /* (a controller in use: its right stick pressed is Z) */
        const char *t = battle_fast ? (padp ? "Back to the chosen speed" : "Back to the chosen speed (Z)") : (padp ? "Fast forward" : "Fast forward (Z)");
        ImDrawList *dl = ImGui::GetWindowDrawList();
        ImVec2 p = ImGui::GetCursorScreenPos(), ts = ImGui::CalcTextSize(t), mp = ImGui::GetIO().MousePos;
        float pad = ImGui::GetTextLineHeight() * 0.3f, gh = ts.y * 1.05f, gw = padp ? ui_pad_glyph_width(gh, PB_RSTICK) + pad : 0;
        float iw = !padp && config_get_int("battle.order_icons", 1) ? ImGui::GetTextLineHeight() * 1.15f : 0;   /* (its picture, as the orders') */
        ImVec2 a = p, b(p.x + ts.x + gw + iw + pad * 2, p.y + ts.y + pad);
        bool hov = mp.x >= a.x && mp.x < b.x && mp.y >= a.y && mp.y < b.y;
        dl->AddRectFilled(a, b, battle_fast ? IM_COL32(150, 60, 30, 255) : hov ? IM_COL32(150, 112, 56, 255) : IM_COL32(96, 70, 36, 255), (b.y - a.y) * 0.2f);
        if (padp) ui_pad_glyph(dl, ImVec2(a.x + pad + (gw - pad) * 0.5f, (a.y + b.y) * 0.5f), gh, PB_RSTICK);
        if (iw > 0) ui_draw_order_icon(dl, ImVec2(a.x + pad + iw * 0.42f, (a.y + b.y) * 0.5f), iw * 0.95f, battle_fast ? OI_RESUME : OI_FAST);
        dl->AddText(ImVec2(a.x + pad + gw + iw, a.y + pad * 0.5f), IM_COL32(244, 232, 204, 255), t);
        mclick(a, b, [] { battle_fast_set(!battle_fast); });
        ImGui::Dummy(ImVec2(b.x - a.x, b.y - a.y + pad));
    }
    order_buttons(width);
    if (!partypanel_shown()) heading("The party");      /* (the party panel drawn anew has them already) */
    for (int i = 0; i < 5 && !partypanel_shown(); i++) {
        u16 b = (u16)(0x9c55 + 0x80 * i);
        int st = g_rb((u16)(b + 0x14));
        if (!st) continue;
        int end = (s8)g_rb((u16)(b + 0x5d)), endm = (s8)g_rb((u16)(b + 0x64)), str = (s8)g_rb((u16)(b + 0x5e)), strm = (s8)g_rb((u16)(b + 0x65));
        const char *cond = st == 3 ? "dead" : st == 2 ? "unconscious" : st == 5 ? "fled" : "";
        if (*cond) ImGui::TextDisabled("%s - %s", member_name(i).c_str(), cond);
        else {
            ImGui::TextUnformatted(member_name(i).c_str());
            ImGui::SameLine(); ImGui::TextDisabled("%d/%d  %d/%d", end, endm, str, strm);
            bar(endm > 0 ? (float)end / endm : 0, IM_COL32(110, 130, 230, 255), bw, bh);
            bar(strm > 0 ? (float)str / strm : 0, IM_COL32(90, 180, 90, 255), bw, bh);
        }
    }
    if (!partypanel_shown()) ImGui::Separator();
    heading("Enemies");
    int shown = 0, down = 0;
    for (int i = 5; i < 13; i++) {
        u16 b = (u16)(0x9c55 + 0x80 * i);
        int st = g_rb((u16)(b + 0x14)), endm = (s8)g_rb((u16)(b + 0x64));
        if (!seen[i] && !battlelog_met(i)) continue;            /* (in sight once, or named in the log) */
        if (!st || endm <= 0) {
            if (last_st[i] >= 2 && !last_name[i].empty()) { ImGui::TextDisabled("%s - %s", last_name[i].c_str(), last_st[i] == 3 ? "dead" : last_st[i] == 5 ? "fled" : "unconscious"); shown++; down++; }
            continue;
        }
        int end = (s8)g_rb((u16)(b + 0x5d)), str = (s8)g_rb((u16)(b + 0x5e)), strm = (s8)g_rb((u16)(b + 0x65));
        std::string nm = battlelog_name(i);
        last_st[i] = st; last_name[i] = nm;
        const char *cond = st == 3 ? "dead" : st == 2 ? "unconscious" : st == 5 ? "fled" : end * 4 <= endm ? "badly hurt" : end * 2 <= endm ? "hurt" : end < endm ? "a little hurt" : "unhurt";
        if (st == 1 && strm > 0 && str * 2 <= strm) cond = "gravely wounded";
        if (st != 1) { ImGui::TextDisabled("%s - %s", nm.c_str(), cond); down++; }
        else {
            ImGui::TextUnformatted(nm.c_str()); ImGui::SameLine(); ImGui::TextDisabled("%s", cond);
            bar((float)end / endm, IM_COL32(200, 90, 70, 255), bw, bh);
            if (g_rb((u16)(GV_cbt_ends_battle + i)) == 1) { ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(214, 120, 90, 255)); ImGui::TextWrapped("The master of this place: once this one has fallen and the fight is over, the place is won."); ImGui::PopStyleColor(); }
        }
        shown++;
    }
    if (!shown) {
        ImGui::TextDisabled("None in sight yet.");
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
        ImGui::TextWrapped("To explore: in Single File (Q) or as a Group (G), click a door, a passage or stairs and the party walks there%s.",
                           !levelmap_available() ? "" : config_get_int("assist.level_map_walk", 1) ? "; Tab shows a map of the level as far as the party has seen it, and a click on the map walks the party there" : "; Tab shows a map of the level as far as the party has seen it");
        ImGui::PopStyleColor();
    }
    else if (down == shown) {
        ImGui::TextDisabled("All down.");
        /* a fight in a street or a dungeon: the party stays there afterwards (combat_flags 1) and the
           game goes on only when it walks out through a way out (s014 combat_over) */
        if ((g_rb(GV_combat_flags) & 1) && !g_rb(GV_real_combat)) {
            ImGui::Spacing();
            ImGui::TextWrapped("The fight is won. To go on, walk the party out of this place: click a way out (the edge of the street, a door, stairs).");
        }
    }
    foes_left_section();
}

/* the text size for what the interface writes over and beside the picture (side panels,
 * tooltips, names on the map): grows with the picture, times display.text_size (%) */

/* no right margin to hold them (a window narrower than 16:9, as a 4:3 screen or 1680 x 1050): the
   order buttons in a strip at the foot of the battlefield while a member is given orders
   (battle.order_bar); their clicks come through margins_click, as the margin's */
extern "C" { float ui_orderbar_top = 0; }       /* the strip's top while shown, else 0 (the battle log keeps above it) */
void order_bar(float x0, float x1, float y1)
{
    ui_orderbar_top = 0;
    if (!battle_showing() || config_get_int("original", 0) || !config_get_int("battle.order_bar", 1) || !menuview_is_on() || g_rb(GV_order_mode) != 1) return;
    float fs = std::max(ui_text_px() * 0.85f, 14.0f), w = (x1 - x0) * 0.94f;
    ImGui::SetNextWindowPos(ImVec2((x0 + x1) * 0.5f, y1 - fs * 0.4f), ImGuiCond_Always, ImVec2(0.5f, 1.0f));
    ImGui::SetNextWindowSizeConstraints(ImVec2(w, 0), ImVec2(w, FLT_MAX));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(fs * 0.6f, fs * 0.5f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(fs * 0.3f, fs * 0.22f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(24, 18, 12, 225));
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(232, 218, 186, 255));
    ImGui::PushStyleColor(ImGuiCol_Separator, IM_COL32(0, 0, 0, 0));
    if (ImGui::Begin("##orderbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize |
                                            ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNav)) {
        ImGui::PushFont(ui_font_overlay);
        ImGui::SetWindowFontScale(fs / ui_font_overlay->FontSize);
        order_buttons(w - fs * 1.2f);
        ImGui::PopFont();
        ui_orderbar_top = ImGui::GetWindowPos().y;
    }
    ImGui::End();
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);
}

