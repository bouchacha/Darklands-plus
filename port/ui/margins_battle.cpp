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
#include "richtext.h"
#include "skillicons.h"
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
void ui_cover(ImVec2 a, ImVec2 b);              /* (ui/margins.cpp: one of our panels over the field) */
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
    case OI_BERSERK: return "Then click an enemy: quick, furious blows without thought of defense.";
    case OI_PARRY: return "Then click an enemy: the member mostly defends, hard to hit, striking back only now and then.";
    case OI_MISSILE: return "Then click an enemy: shoot or throw at it with the missile weapon (a clear line needed).";
    }
    return nullptr;
}
/* the battle menus' Attack and Orders entries (s016 1ef8's), each with what it is: kind (ordericons.h),
   a potion's picture, its label */
struct OrderEntry { int j, k, s2, kind, pid, pq, count = 0; std::string label, key; };
/* the orders as pictures alone (battle.order_icons): the words in the tooltip */
static bool icons_only(void) { return config_get_int("battle.order_icons", 1) != 0; }
/* a thing among the orders that is not a potion (a lantern, a rope ...): its picture; -1 none */
static int item_of(const std::string &name)
{
    for (int id = 0; id < (int)gamedata.items.size(); id++)
        if (!gamedata.items[id].shortname.empty() && (name == gamedata.items[id].shortname || name == gamedata.items[id].name)) return id;
    return -1;
}
static std::vector<OrderEntry> order_entries(void)
{
    std::vector<OrderEntry> v;
    const MenuModel *M = menuview_live();
    if (!M) return v;
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
                OrderEntry o; o.j = j; o.k = k; o.s2 = s2; o.pq = -1;
                o.label = game_text(e.name, strlen(e.name));
                if (s2 >= 0) o.label = game_text(it.name, strlen(it.name)) + ": " + (e.count > 1 ? std::to_string(e.count) + " " : "") + o.label;
                o.pid = s2 >= 0 ? potion_of(game_text(e.name, strlen(e.name)), e.key, &o.pq) : -1;
                if (s2 >= 0 && o.pid < 0) { o.pid = item_of(game_text(e.name, strlen(e.name))); o.pq = -1; }
                o.count = e.count;
                o.kind = s2 < 0 ? ui_order_icon_for(game_text(e.name, strlen(e.name)).c_str()) : OI_NONE;
                if (o.kind == OI_ATTACK) o.label = "Standard";                  /* (the game's "Std Attack") */
                if (e.key[0] && o.pq < 0) o.key = e.key;
                if (e.key[0] && o.pid < 0) o.label += std::string(" (") + e.key + ")";   /* (the key, also on the button's corner; a potion's quality: the dots on its picture) */
                else if (e.key[0] && o.pq >= 0) o.label += std::string(", quality ") + (e.key[0] == 'q' ? e.key + 1 : e.key);
                v.push_back(o);
            }
        }
    }
    return v;
}
/* one order button at a (its size from its label); hovered: its tip. The click comes through
   margins_order_click */
/* a button: in icons_only, a square with the order's picture (a potion's or a thing's, with how many), the
   words under the pointer; else the picture and the words */
static bool pictured(const OrderEntry &e) { return e.pid >= 0 || e.kind != OI_NONE; }
static ImVec2 order_button_size(const OrderEntry &e, bool words = false)
{
    float lh = ImGui::GetTextLineHeight(), pad = lh * 0.3f;
    if (!words && icons_only() && pictured(e)) return ImVec2(lh * 2.0f, lh * 2.0f);
    float iw = pictured(e) ? lh * 1.15f : 0;
    return ImVec2(ImGui::CalcTextSize(e.label.c_str()).x + pad * 2 + iw, lh + pad);
}
static void order_button(ImDrawList *dl, ImVec2 a, const OrderEntry &e, bool enabled = true, bool words = false)
{
    float lh = ImGui::GetTextLineHeight(), pad = lh * 0.3f;
    bool square = !words && icons_only() && pictured(e);
    float iw = square ? lh * 1.55f : pictured(e) ? lh * 1.15f : 0;
    ImVec2 sz = order_button_size(e, words), b(a.x + sz.x, a.y + sz.y), mp = ImGui::GetIO().MousePos;
    bool hov = enabled && mp.x >= a.x && mp.x < b.x && mp.y >= a.y && mp.y < b.y;
    float h = sz.y;
    dl->AddRectFilled(a, b, !enabled ? IM_COL32(70, 58, 42, 200) : hov ? IM_COL32(150, 112, 56, 255) : IM_COL32(96, 70, 36, 255), h * 0.2f);
    dl->AddRect(a, b, IM_COL32(170, 132, 70, enabled ? 255 : 120), h * 0.2f);
    ImVec2 ic = square ? ImVec2((a.x + b.x) * 0.5f, (a.y + b.y) * 0.5f) : ImVec2(a.x + pad + iw * 0.45f, a.y + h * 0.5f);
    int v0 = dl->VtxBuffer.Size;
    if (e.pid >= 0) ui_item_icon_q(dl, ic, iw, e.pid, e.pq);
    else if (e.kind != OI_NONE) ui_draw_order_icon(dl, ic, iw * 0.95f, e.kind);
    if (!enabled) ui_dim_since(dl, v0, 0.45f);
    if (square && e.count > 1) {                                  /* (how many: in the top corner, the quality's marks at the foot) */
        char n[12]; snprintf(n, sizeof n, "%d", e.count);
        float fs = ImGui::GetFontSize() * 0.8f; ImVec2 ts = ImGui::GetFont()->CalcTextSizeA(fs, FLT_MAX, 0, n);
        ImVec2 p(b.x - ts.x - pad * 0.45f, a.y + pad * 0.15f);
        dl->AddText(ImGui::GetFont(), fs, ImVec2(p.x + 1, p.y + 1), IM_COL32(0, 0, 0, 220), n);
        dl->AddText(ImGui::GetFont(), fs, p, IM_COL32(250, 236, 200, 255), n);
    }
    if (square && !e.key.empty() && e.pq < 0) {                   /* (its key, as the menu has it: in the lower left corner, in the plain face) */
        ImFont *kf = ui_font_text ? ui_font_text : ImGui::GetFont();
        float fs = ImGui::GetFontSize() * 0.72f;
        ImVec2 ts = kf->CalcTextSizeA(fs, FLT_MAX, 0, e.key.c_str()), p(a.x + pad * 0.35f, b.y - ts.y - pad * 0.1f);
        dl->AddRectFilled(ImVec2(p.x - 1.5f, p.y), ImVec2(p.x + ts.x + 1.5f, p.y + ts.y), IM_COL32(20, 14, 8, enabled ? 170 : 110), 2.0f);
        dl->AddText(kf, fs, p, enabled ? IM_COL32(240, 222, 170, 245) : IM_COL32(150, 136, 110, 200), e.key.c_str());
    }
    if (!square) dl->AddText(ImVec2(a.x + pad + iw, a.y + pad * 0.5f), enabled ? IM_COL32(244, 232, 204, 255) : IM_COL32(160, 146, 120, 255), e.label.c_str());
    if (enabled && e.j >= 0) order_btns.push_back({ a, b, e.j, e.k, e.s2 });
    if (!enabled && square && mp.x >= a.x && mp.x < b.x && mp.y >= a.y && mp.y < b.y) {   /* (a place kept for an order not open now: what it is, and that) */
        if (a.y > ImGui::GetIO().DisplaySize.y * 0.5f) { ImVec2 wp = ImGui::GetWindowPos(), ws = ImGui::GetWindowSize(); ui_tip_beside(ImVec2(a.x, wp.y), ImVec2(b.x, wp.y + ws.y)); }
        else ui_tip_beside(a, b);
        margin_tip((e.label + "\nNot open to this member now.").c_str());
    }
    if (hov && (square || order_tip(e.kind))) {   /* (beside the button; in the strip at the field's foot, above the whole strip, not over its buttons) */
        if (a.y > ImGui::GetIO().DisplaySize.y * 0.5f) { ImVec2 wp = ImGui::GetWindowPos(), ws = ImGui::GetWindowSize(); ui_tip_beside(ImVec2(a.x, wp.y), ImVec2(b.x, wp.y + ws.y)); }   /* (the strip's whole height: above it) */
        else ui_tip_beside(a, b);
        std::string t = square ? e.label + (order_tip(e.kind) ? std::string("\n") + order_tip(e.kind) : std::string()) : order_tip(e.kind);
        margin_tip(t.c_str());
    }
}
/* the selected member's orders as buttons, in places that do not move: every member's strip the same, an
   order not open to this one dimmed in its place (in a fight: the four attacks and Use Missile, Walk, Flee
   and Halt; walking a level: Walk, Halt, as a group, single file); then Surrender, Cancel (and Resume when
   with_resume); the orders of the place the member stands in (a door, stairs, a chest, the loot ...) last,
   so that they move nothing else. The potions and things to use, from none to dozens, are in a grid of their
   own above the strip (potions_tray), always in sight: they move nothing in it. In words (battle.order_icons
   off) the same order */
static int bar_frame = -100;                      /* the frame the strip last listed its buttons */
static std::vector<OrderEntry> tray;              /* the potions and things to throw or use, for the grid */
static const char *SLOT_NAME(int kind)
{
    switch (kind) {
    case OI_ATTACK: return "Standard"; case OI_VULNERABLE: return "Vulnerable"; case OI_BERSERK: return "Berserk";
    case OI_PARRY: return "Parry"; case OI_MISSILE: return "Use Missile"; case OI_WALK: return "Walk Towards";
    case OI_FLEE: return "Flee Towards"; case OI_HALT: return "Halt"; case OI_GROUP: return "Travel As Group";
    case OI_FILE: return "Travel Single File"; case OI_SURRENDER: return "Surrender"; case OI_CANCEL: return "Cancel Giving Order";
    }
    return "";
}
static void order_buttons(float width, bool with_resume)
{
    order_btns.clear();
    tray.clear();
    if (!menuview_is_on() || g_rb(GV_order_mode) != 1) return;
    int sel = (s8)g_rb(GV_selected_member);
    if (sel < 0 || sel > 4) return;
    char nm[32]; { u16 b = G_MEMBER(sel); int i = 0; for (; i < 31; i++) { char c = (char)g_rb((u16)(b + 0x2a + i)); if (!c) break; nm[i] = c; } nm[i] = 0; }
    std::string who = game_text(nm, strlen(nm)); size_t sp = who.find(' '); if (sp != std::string::npos) who = who.substr(0, sp);
    std::vector<OrderEntry> all = order_entries(), here, last;
    tray.clear();
    const bool fight = g_rb(GV_real_combat) == 1;
    static const int FIGHT[] = { OI_ATTACK, OI_VULNERABLE, OI_BERSERK, OI_PARRY, OI_MISSILE, -1, OI_WALK, OI_FLEE, OI_HALT, -1 };
    static const int WALKS[] = { OI_WALK, OI_HALT, OI_GROUP, OI_FILE, -1 };
    const int *slots = fight ? FIGHT : WALKS; int nslots = fight ? 10 : 5;
    std::vector<OrderEntry> row;                 /* (the strip, in its order; kind OI_NONE with j -2: a rule between groups) */
    auto find = [&](int kind) -> OrderEntry * { for (OrderEntry &e : all) if (e.s2 < 0 && e.kind == kind) return &e; return nullptr; };
    static std::string key_of[OI_N];                 /* (an order's key as last seen in a menu: on its dimmed place too) */
    for (OrderEntry &e : all) if (e.s2 < 0 && e.kind >= 0 && e.kind < OI_N) { if (e.kind == OI_CANCEL && e.key.empty()) e.key = "Esc"; if (!e.key.empty()) key_of[e.kind] = e.key; }
    auto placeholder = [&](int kind) { OrderEntry o{ -1, -1, -1, kind, -1, -1, 0, SLOT_NAME(kind), kind >= 0 && kind < OI_N ? key_of[kind] : "" }; return o; };
    auto rule = [&]() { OrderEntry o{ -2, -1, -1, OI_NONE, -1, -1, 0, "", "" }; row.push_back(o); };
    for (int i = 0; i < nslots; i++) {
        if (slots[i] < 0) { rule(); continue; }
        OrderEntry *e = find(slots[i]);
        row.push_back(e ? *e : placeholder(slots[i]));
    }
    for (const OrderEntry &e : all) {
        if ((e.kind == OI_INFO && ui_battle_info) || (e.kind == OI_DONE && config_get_int("battle.quick_orders", 1))) continue;
        if (e.kind == OI_RESUME && !with_resume) continue;
        if (e.s2 >= 0) { tray.push_back(e); continue; }
        bool slotted = false; for (int i = 0; i < nslots; i++) if (slots[i] == e.kind) slotted = true;
        if (slotted || e.kind == OI_SURRENDER || e.kind == OI_CANCEL) continue;
        switch (e.kind) {
        case OI_DOOR: case OI_STAIRS: case OI_CHEST: case OI_PICK: case OI_DISSOLVE: case OI_DISARM: case OI_LOOT: case OI_EXIT: here.push_back(e); break;
        default: last.push_back(e); break;                   /* (Resume, Enemy Info, Done; one of the attacks out of a fight ...) */
        }
    }
    std::sort(tray.begin(), tray.end(), [](const OrderEntry &a, const OrderEntry &b) { return a.pid != b.pid ? a.pid < b.pid : a.pq < b.pq; });
    { OrderEntry *e = find(OI_SURRENDER); if (fight || e) row.push_back(e ? *e : placeholder(OI_SURRENDER)); }
    { OrderEntry *e = find(OI_CANCEL); row.push_back(e ? *e : placeholder(OI_CANCEL)); }
    for (const OrderEntry &e : last) row.push_back(e);
    if (!here.empty()) { rule(); for (const OrderEntry &e : here) row.push_back(e); }

    const bool ic = icons_only();
    if (!ic) heading(("Orders for " + who).c_str());
    ImDrawList *dl = ImGui::GetWindowDrawList();
    ImVec2 o0 = ImGui::GetCursorScreenPos();
    float lh = ImGui::GetTextLineHeight(), gap = lh * 0.25f, ggap = ic ? lh * 0.7f : lh * 0.9f;
    float x0 = o0.x, x1 = o0.x + width - ImGui::GetStyle().WindowPadding.x * 2, x = x0, y = o0.y, rowh = 0, xmax = x0;
    if (ic) {                                                     /* (whose orders: their head, the name under the pointer) */
        float hs = lh * 2.0f;
        ImVec2 h0(x0, y), h1(x0 + hs / 1.2f, y + hs);
        ui_member_head(dl, sel, h0, hs);
        if (ImGui::IsMouseHoveringRect(h0, h1, false)) { ui_tip_beside(h0, h1); margin_tip(("Orders for " + who).c_str()); }
        x = x0 = h1.x + ggap * 0.6f; xmax = x;
    }
    for (const OrderEntry &e : row) {
        if (e.j == -2) {                                          /* (between groups: a thin rule) */
            if (x > x0) { float bh = ic ? lh * 2.0f : lh * 1.3f; dl->AddLine(ImVec2(x - gap + ggap * 0.5f, y + bh * 0.15f), ImVec2(x - gap + ggap * 0.5f, y + bh * 0.85f), IM_COL32(170, 132, 70, 150), 1.0f); x += ggap - gap; }
            continue;
        }
        ImVec2 bs = order_button_size(e);
        if (x > x0 && x + bs.x > x1) { x = x0; y += rowh + gap; rowh = 0; }   /* (too narrow a window: on to the next row) */
        order_button(dl, ImVec2(x, y), e, e.j >= 0);
        x += bs.x + gap; rowh = fmaxf(rowh, bs.y); xmax = fmaxf(xmax, x - gap);
    }
    ImGui::SetCursorScreenPos(ImVec2(x0, y + (rowh > 0 ? rowh + gap : 0)));
    ImGui::Dummy(ImVec2(xmax - x0, 1));                    /* (the strip as wide as its buttons: the field shows round it) */
    order_btns_frame = bar_frame = ImGui::GetFrameCount();
}
/* the potions (and things to use) of the member given orders, above the strip whenever it shows: what they
   carry stays in sight. A grid in the same order for every member (by kind, then quality), a size smaller
   than the strip's buttons, as many to a row as the strip is wide, the rows growing upwards from the strip
   (dozens of kinds take a few rows; none, no grid). A click throws or uses one (the order as the game's
   menu gives it) */
static void potions_tray(float cx, float bottom, float fs, float maxw)
{
    if (tray.empty()) return;
    ImGui::SetNextWindowPos(ImVec2(cx, bottom), ImGuiCond_Always, ImVec2(0.5f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(fs * 0.6f, fs * 0.5f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(24, 18, 12, 220));
    if (ImGui::Begin("##potiontray", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize |
                                               ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNav)) {
        ImGui::PushFont(ui_font_overlay);
        ImGui::SetWindowFontScale(fs * 0.82f / ui_font_overlay->FontSize);
        ImDrawList *dl = ImGui::GetWindowDrawList();
        ImVec2 o0 = ImGui::GetCursorScreenPos();
        float lh = ImGui::GetTextLineHeight(), gap = lh * 0.25f, x1 = o0.x + maxw - fs * 1.2f;
        float x = o0.x, y = o0.y, xmax = o0.x, rowh = 0;
        for (size_t i = 0; i < tray.size(); i++) {
            ImVec2 bs = order_button_size(tray[i]);
            if (x > o0.x && x + bs.x > x1) { x = o0.x; y += rowh + gap; rowh = 0; }
            const OrderEntry &e = tray[i];
            order_button(dl, ImVec2(x, y), e);
            if (e.j >= 0) { int m = e.j, k = e.k, s2 = e.s2; mclick(ImVec2(x, y), ImVec2(x + bs.x, y + bs.y), [m, k, s2] { menuview_request(m, k, s2); }); }
            x += bs.x + gap; rowh = fmaxf(rowh, bs.y); xmax = fmaxf(xmax, x - gap);
        }
        ImGui::SetCursorScreenPos(ImVec2(o0.x, y + rowh));
        ImGui::Dummy(ImVec2(xmax - o0.x, 1));
        ImGui::PopFont();
        { ImVec2 p = ImGui::GetWindowPos(), z = ImGui::GetWindowSize(); ui_cover(p, ImVec2(p.x + z.x, p.y + z.y)); }
    }
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
}
/* the top of the right margin in a battle: fast forward (Z), and Resume (Space) under it, always in
   their places (Resume dimmed while the battle runs) */
static void top_buttons(void)
{
    const bool padp = ui_pad_prompts();               /* (a controller in use: its right stick pressed is Z) */
    ImDrawList *dl = ImGui::GetWindowDrawList();
    ImVec2 p = ImGui::GetCursorScreenPos(), mp = ImGui::GetIO().MousePos;
    float lh = ImGui::GetTextLineHeight(), pad = lh * 0.3f, gap = lh * 0.4f;
    float bx = p.x;
    if (config_get_int("battle.fast_key", 1) && game_mode() == GM_BATTLE) {   /* fast forward (Z, mods/pathfix.c): on the field (not over a sheet or the loot) */
        const char *t = battle_fast ? (padp ? "Back to the chosen speed" : "Back to the chosen speed (Z)") : (padp ? "Fast forward" : "Fast forward (Z)");
        ImVec2 ts = ImGui::CalcTextSize(t);
        float gh = ts.y * 1.05f, gw = padp ? ui_pad_glyph_width(gh, PB_RSTICK) + pad : 0;
        float iw = !padp && config_get_int("battle.order_icons", 1) ? lh * 1.15f : 0;   /* (its picture, as the orders') */
        ImVec2 a = p, b(p.x + ts.x + gw + iw + pad * 2, p.y + ts.y + pad);
        bool hov = mp.x >= a.x && mp.x < b.x && mp.y >= a.y && mp.y < b.y;
        dl->AddRectFilled(a, b, battle_fast ? IM_COL32(150, 60, 30, 255) : hov ? IM_COL32(150, 112, 56, 255) : IM_COL32(96, 70, 36, 255), (b.y - a.y) * 0.2f);
        if (padp) ui_pad_glyph(dl, ImVec2(a.x + pad + (gw - pad) * 0.5f, (a.y + b.y) * 0.5f), gh, PB_RSTICK);
        if (iw > 0) ui_draw_order_icon(dl, ImVec2(a.x + pad + iw * 0.42f, (a.y + b.y) * 0.5f), iw * 0.95f, battle_fast ? OI_RESUME : OI_FAST);
        dl->AddText(ImVec2(a.x + pad + gw + iw, a.y + pad * 0.5f), IM_COL32(244, 232, 204, 255), t);
        mclick(a, b, [] { battle_fast_set(!battle_fast); });
        p.y = b.y + gap;
    }
    if (menuview_is_on() && game_mode() == GM_BATTLE) {           /* Resume: the Orders menu's own entry, given as a click on it */
        bool paused = g_rb(GV_order_mode) == 1;
        OrderEntry r{ -1, -1, -1, OI_RESUME, -1, -1, 0, padp ? "Resume" : "Resume (Space)", "" };
        if (paused) for (const OrderEntry &e : order_entries()) if (e.kind == OI_RESUME) { r.j = e.j; r.k = e.k; r.s2 = e.s2; }
        bool ok = paused && r.j >= 0;
        if (bar_frame != ImGui::GetFrameCount()) order_btns.clear();   /* (no order strip this frame: the list holds Resume alone) */
        order_button(dl, ImVec2(bx, p.y), r, ok, true);              /* (in the margin, with its words) */
        if (ok) order_btns_frame = ImGui::GetFrameCount();
        ImVec2 rs = order_button_size(r, true);
        if (!ok && mp.x >= bx && mp.x < bx + rs.x && mp.y >= p.y && mp.y < p.y + rs.y) ui_tip_beside(ImVec2(bx, p.y), ImVec2(bx + rs.x, p.y + rs.y)), margin_tip("The battle is running: Space pauses it, or click one of the party.");
        p.y += rs.y + gap;
    }
    ImGui::SetCursorScreenPos(ImVec2(bx, p.y));
    ImGui::Dummy(ImVec2(1, pad));
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
    if (!config_get_int("original", 0)) top_buttons();
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
        ImGui::TextWrapped("To explore: in Single File (Q) or as a Group (G), click a passage or stairs and the party walks there; click a shut door and they walk to it and open it%s.",
                           !levelmap_available() ? "" : "; Tab shows the level map (as far as the party has seen it), and a click on it walks the party there");
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
    margin_effects(gm_party_size(), true);               /* (what is at work goes on in a fight: as in the town's margin) */
}

/* the text size for what the interface writes over and beside the picture (side panels,
 * tooltips, names on the map): grows with the picture, times display.text_size (%) */

/* no right margin to hold them (a window narrower than 16:9, as a 4:3 screen or 1680 x 1050): the
   order buttons in a strip at the foot of the battlefield while a member is given orders
   (battle.order_bar); their clicks come through margins_click, as the margin's */
extern "C" { float ui_orderbar_top = 0; }       /* the strip's top while shown, else 0 (the battle log keeps above it) */
void order_bar(float x0, float x1, float y1, bool with_resume)
{
    ui_orderbar_top = 0;
    if (!battle_showing() || game_mode() != GM_BATTLE || config_get_int("original", 0) || !config_get_int("battle.order_bar", 1) || !menuview_is_on() || g_rb(GV_order_mode) != 1) return;   /* (on the field: not over a member's sheet opened in the battle, where it stood empty) */
    float fs = std::max(ui_text_px() * 0.85f, 14.0f), w = (x1 - x0) * 0.94f;
    ImGui::SetNextWindowPos(ImVec2((x0 + x1) * 0.5f, y1 - fs * 0.4f), ImGuiCond_Always, ImVec2(0.5f, 1.0f));
    ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(w, FLT_MAX));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(fs * 0.6f, fs * 0.5f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(fs * 0.3f, fs * 0.22f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(24, 18, 12, 190));
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(232, 218, 186, 255));
    ImGui::PushStyleColor(ImGuiCol_Separator, IM_COL32(0, 0, 0, 0));
    if (ImGui::Begin("##orderbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize |
                                            ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNav)) {
        ImGui::PushFont(ui_font_overlay);
        ImGui::SetWindowFontScale(fs / ui_font_overlay->FontSize);
        order_buttons(w - fs * 1.2f, with_resume);
        ImGui::PopFont();
        ui_orderbar_top = ImGui::GetWindowPos().y;
        { ImVec2 p = ImGui::GetWindowPos(), z = ImGui::GetWindowSize(); ui_cover(p, ImVec2(p.x + z.x, p.y + z.y)); }   /* (the pointer on it: not on the field) */
    }
    ImGui::End();
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);
    if (ui_orderbar_top > 0) potions_tray((x0 + x1) * 0.5f, ui_orderbar_top - fs * 0.3f, fs, w);
}

