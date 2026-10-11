// inventory.cpp - the graphical inventory and the alchemy overview of the Darklands+ menu.
// Items are drawn as procedural icons by kind (weapons, armour, ingredients, potions ...);
// dragging an item onto another member gives it to them through the game's own routine
// (give_stack, s031 18e7:007c), so equipment, weights and stacking stay right.
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <string>
#include <vector>
#include "imgui.h"
#include "gamedata.h"
extern "C" {
#include "../runtime/game.h"
}

extern ImFont *ui_font_small;

struct InvItem { int idx, id, type, qual, count, weight; bool equipped; };

static const int MAXM = 5;
static int sel_member = 0;
static int kind_filter = -1;
static std::string last_msg;

static bool member_present(int m)
{
    if (m >= (s16)g_rw(G_PARTY_SIZE)) return false;
    char nm[4]; g_str(G_DGROUP, (u16)(G_MEMBER(m) + G_M_FULLNAME), nm, sizeof nm);
    return nm[0] != 0;
}
static std::string member_name(int m, bool full)
{
    char nm[32]; g_str(G_DGROUP, (u16)(G_MEMBER(m) + (full ? G_M_FULLNAME : G_M_SHORTNAME)), nm, sizeof nm);
    return nm;
}

static std::vector<InvItem> member_items(int m)
{
    std::vector<InvItem> v;
    int n = g_rb((u16)(G_MEMBER(m) + G_M_NITEMS));
    if (n > 64) n = 64;
    u16 r0 = G_MEMBER(m);
    int eq_type[5] = { (s8)g_rb((u16)(r0 + 0x3d)), (s8)g_rb((u16)(r0 + 0x0e)), (s8)g_rb((u16)(r0 + 0x37)), (s8)g_rb((u16)(r0 + 0x38)), (s8)g_rb((u16)(r0 + 0x48)) };
    int eq_q[5] = { g_rb((u16)(r0 + 0x44)), g_rb((u16)(r0 + 0x46)), g_rb((u16)(r0 + 0x3b)), g_rb((u16)(r0 + 0x3c)), g_rb((u16)(r0 + 0x47)) };
    for (int i = 0; i < n; i++) {
        u16 r = G_ITEM(m, i);
        InvItem it;
        it.idx = i; it.id = rw(G_FAR2E38, r); it.type = rb(G_FAR2E38, (u16)(r + 2)); it.qual = rb(G_FAR2E38, (u16)(r + 3));
        it.count = rb(G_FAR2E38, (u16)(r + 4)); it.weight = rb(G_FAR2E38, (u16)(r + 5));
        it.equipped = false;
        for (int k = 0; k < 5; k++) if (eq_type[k] >= 0 && eq_type[k] == it.type && eq_q[k] == it.qual) it.equipped = true;
        v.push_back(it);
    }
    return v;
}

static const ItemDef *def_of(int id)
{
    if (id < 0 || id >= (int)gamedata.items.size()) return nullptr;
    return &gamedata.items[id];
}

/* ---------------------------------------------------------------- icons */
static ImU32 col(float r, float g, float b, float a = 1) { return ImGui::ColorConvertFloat4ToU32(ImVec4(r, g, b, a)); }

struct Tint { const char *name; float r, g, b; };
static const Tint TINTS[] = {
    { "Pure Gold", 0.95f, 0.78f, 0.20f }, { "Manganes", 0.38f, 0.30f, 0.42f }, { "Naptha", 0.45f, 0.30f, 0.10f },
    { "Marsh Vapor", 0.55f, 0.70f, 0.45f }, { "East. Black Bean", 0.16f, 0.12f, 0.10f }, { "Zincblende", 0.55f, 0.38f, 0.22f },
    { "Antimoni", 0.70f, 0.72f, 0.76f }, { "Orpiment", 0.97f, 0.65f, 0.15f }, { "White Cinnabar", 0.95f, 0.88f, 0.90f },
    { "Solanaceae", 0.48f, 0.22f, 0.55f }, { "Aqua Regia", 0.95f, 0.62f, 0.18f }, { "Gum", 0.82f, 0.55f, 0.20f },
    { "Mondragora", 0.55f, 0.40f, 0.25f }, { "Alum", 0.92f, 0.94f, 0.96f }, { "Camomile", 0.98f, 0.92f, 0.55f },
    { "Nikel", 0.62f, 0.66f, 0.62f }, { "Pitchblende", 0.18f, 0.24f, 0.18f }, { "Zinken", 0.52f, 0.60f, 0.70f },
    { "Brimstone", 0.95f, 0.88f, 0.20f }, { "Choleric Base", 0.90f, 0.35f, 0.15f }, { "Phleqmatic Base", 0.30f, 0.50f, 0.90f },
    { "Sanguine Base", 0.80f, 0.12f, 0.15f }, { "Melanc. Base", 0.25f, 0.20f, 0.18f },
};

static ImVec4 tint_for(const ItemDef *d, int id)
{
    if (d) for (const Tint &t : TINTS) if (d->name == t.name) return ImVec4(t.r, t.g, t.b, 1);
    // stable pseudo-random hue from the id
    float h = fmodf(id * 0.61803f, 1.0f), r, g, b;
    ImGui::ColorConvertHSVtoRGB(h, 0.55f, 0.85f, r, g, b);
    return ImVec4(r, g, b, 1);
}

static bool is_liquid(const ItemDef *d)
{
    if (!d) return false;
    return d->name == "Naptha" || d->name == "Aqua Regia" || d->name.find("Base") != std::string::npos;
}
static bool is_plant(const ItemDef *d)
{
    if (!d) return false;
    return d->name == "Solanaceae" || d->name == "Mondragora" || d->name == "Camomile" || d->name == "Gum" || d->name == "East. Black Bean";
}

static void draw_icon(ImDrawList *dl, ImVec2 c, float s, const ItemDef *d, int id)
{
    ItemKind k = d ? d->kind : K_OTHER;
    ImU32 ink = col(0.22f, 0.14f, 0.08f), steel = col(0.72f, 0.74f, 0.78f), dark = col(0.35f, 0.25f, 0.15f);
    ImVec4 t = tint_for(d, id);
    ImU32 tc = ImGui::ColorConvertFloat4ToU32(t), tl = col(fminf(1, t.x + 0.25f), fminf(1, t.y + 0.25f), fminf(1, t.z + 0.25f));
    float u = s / 16.0f;
    auto P = [&](float x, float y) { return ImVec2(c.x + x * u, c.y + y * u); };
    switch (k) {
    case K_WEAPON: case K_MISSILE:
        if (k == K_MISSILE && d && (d->flags & 0x20)) {          // bows
            dl->PathArcTo(P(-2, 0), 9 * u, -1.2f, 1.2f, 16); dl->PathStroke(dark, 0, 2.2f * u);
            dl->AddLine(P(2.8f, -8.4f), P(2.8f, 8.4f), ink, 0.8f * u);
        } else {
            dl->AddQuadFilled(P(-1, 6), P(1, 6), P(1, -9), P(-1, -9), steel);
            dl->AddTriangleFilled(P(-1, -9), P(1, -9), P(0, -11.5f), steel);
            dl->AddRectFilled(P(-4.5f, 5.5f), P(4.5f, 7), dark);
            dl->AddRectFilled(P(-0.8f, 7), P(0.8f, 11), col(0.45f, 0.30f, 0.18f));
            dl->AddCircleFilled(P(0, 11.5f), 1.4f * u, col(0.75f, 0.6f, 0.2f));
        }
        break;
    case K_ARMOR:
        dl->AddQuadFilled(P(-7, -8), P(7, -8), P(6, 9), P(-6, 9), steel);
        dl->AddQuadFilled(P(-10, -8), P(-7, -8), P(-7, -2), P(-10, -3), steel);
        dl->AddQuadFilled(P(7, -8), P(10, -8), P(10, -3), P(7, -2), steel);
        for (int r = -5; r < 9; r += 3) dl->AddLine(P(-6, (float)r), P(6, (float)r), col(0.5f, 0.52f, 0.56f), 0.7f * u);
        break;
    case K_SHIELD:
        dl->PathLineTo(P(-8, -9)); dl->PathLineTo(P(8, -9)); dl->PathLineTo(P(8, 1)); dl->PathLineTo(P(0, 11)); dl->PathLineTo(P(-8, 1));
        dl->PathFillConvex(col(0.62f, 0.18f, 0.12f));
        dl->AddLine(P(0, -9), P(0, 11), col(0.9f, 0.8f, 0.3f), 1.6f * u);
        dl->AddLine(P(-8, -3), P(8, -3), col(0.9f, 0.8f, 0.3f), 1.6f * u);
        break;
    case K_INGREDIENT:
        if (is_liquid(d)) {
            dl->AddRectFilled(P(-2, -10), P(2, -6), col(0.75f, 0.7f, 0.6f));
            dl->AddCircleFilled(P(0, 3), 7.5f * u, col(0.85f, 0.9f, 0.95f, 0.5f));
            dl->AddCircleFilled(P(0, 4), 6.5f * u, tc);
            dl->AddCircle(P(0, 3), 7.5f * u, ink, 0, 1.0f * u);
        } else if (is_plant(d)) {
            dl->AddBezierQuadratic(P(-6, 9), P(-6, -6), P(5, -9), col(0.30f, 0.45f, 0.20f), 1.6f * u);
            dl->AddCircleFilled(P(-3, -1), 4 * u, tc);
            dl->AddCircleFilled(P(3, -6), 3.5f * u, tc);
            dl->AddCircleFilled(P(2, 3), 3 * u, tl);
        } else if (d && d->name == "Marsh Vapor") {
            dl->AddCircleFilled(P(-4, 2), 5 * u, tc); dl->AddCircleFilled(P(3, -1), 6 * u, tc); dl->AddCircleFilled(P(4, 5), 4 * u, tl);
        } else {                                                  // minerals: crystals
            dl->AddTriangleFilled(P(-8, 9), P(-3, -7), P(1, 9), tc);
            dl->AddTriangleFilled(P(-1, 9), P(4, -10), P(9, 9), tl);
            dl->AddTriangle(P(-1, 9), P(4, -10), P(9, 9), ink, 0.8f * u);
            dl->AddTriangle(P(-8, 9), P(-3, -7), P(1, 9), ink, 0.8f * u);
        }
        break;
    case K_POTION:
        dl->AddRectFilled(P(-2, -11), P(2, -5), col(0.80f, 0.75f, 0.65f));
        dl->AddRectFilled(P(-2.5f, -12), P(2.5f, -10), col(0.55f, 0.35f, 0.20f));
        dl->AddCircleFilled(P(0, 3), 8 * u, col(0.85f, 0.92f, 0.98f, 0.55f));
        dl->PathArcTo(P(0, 3), 7 * u, 0.15f, 3.0f, 20); dl->PathFillConvex(tc);
        dl->AddCircle(P(0, 3), 8 * u, ink, 0, 1.0f * u);
        dl->AddCircleFilled(P(-3, 0), 1.5f * u, col(1, 1, 1, 0.7f));
        break;
    case K_RELIC:
        dl->AddCircle(P(0, -2), 9 * u, col(0.95f, 0.8f, 0.3f), 0, 1.4f * u);
        dl->AddRectFilled(P(-1.2f, -8), P(1.2f, 10), col(0.85f, 0.65f, 0.15f));
        dl->AddRectFilled(P(-5, -4), P(5, -1.6f), col(0.85f, 0.65f, 0.15f));
        break;
    case K_HORSE:
        dl->PathArcTo(P(0, 1), 8 * u, -3.6f, 0.45f, 20); dl->PathStroke(steel, 0, 3 * u);
        break;
    case K_DOCUMENT: case K_QUEST:
        dl->AddRectFilled(P(-7, -9), P(7, 9), col(0.95f, 0.90f, 0.75f));
        dl->AddRect(P(-7, -9), P(7, 9), ink, 0, 0, 0.8f * u);
        for (int r = -6; r < 7; r += 3) dl->AddLine(P(-5, (float)r), P(5, (float)r), col(0.4f, 0.3f, 0.2f), 0.6f * u);
        if (k == K_DOCUMENT) dl->AddCircleFilled(P(4, 7), 2.5f * u, col(0.7f, 0.1f, 0.1f));
        break;
    case K_INSTRUMENT:
        dl->AddCircleFilled(P(-2, 4), 6 * u, col(0.62f, 0.42f, 0.22f));
        dl->AddRectFilled(P(1, -10), P(3, 2), col(0.45f, 0.30f, 0.15f));
        break;
    default:                                                      // gear, tools, other
        dl->AddRectFilled(P(-7, -5), P(7, 8), col(0.55f, 0.40f, 0.25f), 2 * u);
        dl->AddRect(P(-7, -5), P(7, 8), ink, 2 * u, 0, 0.8f * u);
        dl->AddLine(P(-7, 0), P(7, 0), ink, 0.8f * u);
        break;
    }
}

/* the icon of item id, for the other tabs */
void ui_item_icon(ImDrawList *dl, ImVec2 c, float s, int id) { draw_icon(dl, c, s, def_of(id), id); }

/* ---------------------------------------------------------------- giving items */
struct DragItem { int from, idx; };

/* Items may change hands only where the game does not hold inventory rows on screen:
 * in a town or encounter card and on the travel map (not in battle, not on the character
 * sheet or the barter screen, whose lists refer to items by row). */
static const char *why_locked(void)
{
    switch (game_mode()) {
    case GM_CARD: case GM_MAP: return nullptr;
    case GM_BATTLE: return "Items can't change hands during a battle.";
    case GM_SCREEN: return "Close the game's current screen (sheet, shop ...) to hand items over.";
    default: return "Start or load a game first.";
    }
}

static bool equippable(const ItemDef *d, int type)
{
    if (!d) return false;
    return (d->flags & 0x0f) || (d->flags & 0x30) || (type >= 0x43 && type <= 0x61);
}

extern "C" void ui_give(int from, int to, int idx);
static void give(int from, int to, int idx, bool one = false)
{
    if (const char *w = why_locked()) { last_msg = w; return; }
    int before = g_rb((u16)(G_MEMBER(to) + G_M_NITEMS));
    if (before >= 64) { last_msg = member_name(to, false) + " cannot carry any more items."; return; }
    u16 args[4] = { (u16)from, (u16)to, (u16)idx, 0 };
    /* the game's own routines (s031): give_one 18e7:0000, give_stack 18e7:007c */
    if (game_call_far(31, 0x18e7, one ? 0x0000 : 0x007c, 4, args) < 0) { last_msg = "Could not give the item."; return; }
    last_msg = std::string(one ? "Gave one to " : "Gave to ") + member_name(to, false) + ".";
}

static void equip(int m, int idx, bool on)
{
    if (const char *w = why_locked()) { last_msg = w; return; }
    if (on) { u16 a[2] = { (u16)m, (u16)idx }; game_call_far(31, 0x18e7, 0x11b2, 2, a); }       /* equip */
    else { u16 a[3] = { (u16)m, (u16)idx, 0 }; game_call_far(31, 0x18e7, 0x12e8, 3, a); }      /* item_unequip */
    last_msg = member_name(m, false) + (on ? " takes it up." : " puts it away.");
}

/* ---------------------------------------------------------------- the inventory tab */
static void member_cards(void)
{
    float w = (ImGui::GetContentRegionAvail().x - 4 * ImGui::GetStyle().ItemSpacing.x) / 5;
    for (int m = 0; m < MAXM; m++) {
        if (!member_present(m)) continue;
        ImGui::PushID(m);
        bool sel = m == sel_member;
        if (sel) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
        else ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.72f, 0.60f, 0.42f, 1));
        int n = g_rb((u16)(G_MEMBER(m) + G_M_NITEMS));
        u16 r0 = G_MEMBER(m);
        int load = g_rb((u16)(r0 + G_M_LOAD)), cap = (s8)g_rb((u16)(r0 + G_M_ATTR_CUR)) + (s8)g_rb((u16)(r0 + G_M_ATTR_CUR + 1));
        char label[96]; snprintf(label, sizeof label, "%s\n%d items  load %d/%d", member_name(m, false).c_str(), n, load, cap);
        ImGui::PushStyleColor(ImGuiCol_Text, sel ? ImVec4(1, 0.95f, 0.85f, 1) : ImGui::GetStyle().Colors[ImGuiCol_Text]);
        if (ImGui::Button(label, ImVec2(w, 58))) sel_member = m;
        ImGui::PopStyleColor(2);
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload *p = ImGui::AcceptDragDropPayload("DL_ITEM")) {
                DragItem di; memcpy(&di, p->Data, sizeof di);
                if (di.from != m) give(di.from, m, di.idx, ImGui::GetIO().KeyShift);
            }
            ImGui::EndDragDropTarget();
        }
        ImGui::PopID();
        ImGui::SameLine();
    }
    ImGui::NewLine();
}

static void tooltip(const InvItem &it, const ItemDef *d)
{
    ImGui::BeginTooltip();
    ImGui::Text("%s", d ? d->name.c_str() : "?");
    ImGui::Separator();
    if (d) ImGui::TextDisabled("%s", kind_name(d->kind));
    ImGui::Text("Quality %d", it.qual);
    if (it.count > 1) ImGui::Text("%d of them", it.count);
    ImGui::Text("Weight %d", it.weight);
    if (d && d->value) ImGui::Text("Worth about %d pfennigs", d->value);
    if (it.equipped) ImGui::TextColored(ImVec4(0.45f, 0.18f, 0.10f, 1), "In use");
    if (d && d->kind == K_POTION) {
        int p = it.id - 95;
        if (p >= 0 && 3 * p < (int)gamedata.formulas.size()) { ImGui::PushTextWrapPos(320); ImGui::TextWrapped("%s", gamedata.formulas[3 * p].text.c_str()); ImGui::PopTextWrapPos(); }
    }
    ImGui::TextDisabled("Drag onto a name to give it (Shift: just one). Right-click for more.");
    ImGui::EndTooltip();
}

extern "C" void tab_inventory(void)
{
    if (!gamedata.loaded) { ImGui::Text("The item list (DARKLAND.LST) could not be read."); return; }
    if (!member_present(sel_member)) for (int m = 0; m < MAXM; m++) if (member_present(m)) { sel_member = m; break; }
    member_cards();
    // kind filter chips
    std::vector<InvItem> items = member_items(sel_member);
    bool present[K_OTHER + 1] = {};
    for (auto &it : items) { const ItemDef *d = def_of(it.id); present[d ? d->kind : K_OTHER] = true; }
    if (ImGui::RadioButton("Everything", kind_filter < 0)) kind_filter = -1;
    for (int k = 0; k <= K_OTHER; k++) if (present[k]) { ImGui::SameLine(); if (ImGui::RadioButton(kind_name((ItemKind)k), kind_filter == k)) kind_filter = k; }
    ImGui::BeginChild("grid", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() - 4), ImGuiChildFlags_Border);
    const float tile = 104, pad = 8;
    int cols = (int)((ImGui::GetContentRegionAvail().x + pad) / (tile + pad)); if (cols < 1) cols = 1;
    int col_i = 0;
    ImDrawList *dl = ImGui::GetWindowDrawList();
    for (auto &it : items) {
        const ItemDef *d = def_of(it.id);
        if (kind_filter >= 0 && (d ? d->kind : K_OTHER) != kind_filter) continue;
        if (col_i) ImGui::SameLine(0, pad);
        ImGui::PushID(it.idx);
        ImVec2 p = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton("tile", ImVec2(tile, tile));
        bool hov = ImGui::IsItemHovered();
        ImU32 bg = it.equipped ? col(0.87f, 0.76f, 0.55f) : col(0.96f, 0.92f, 0.82f);
        dl->AddRectFilled(p, ImVec2(p.x + tile, p.y + tile), bg, 6);
        if (it.equipped) dl->AddRect(ImVec2(p.x + 3, p.y + 3), ImVec2(p.x + tile - 3, p.y + tile - 3), col(0.72f, 0.55f, 0.18f), 4, 0, 2.0f);
        dl->AddRect(p, ImVec2(p.x + tile, p.y + tile), hov ? col(0.55f, 0.24f, 0.13f) : col(0.55f, 0.42f, 0.25f), 6, 0, hov ? 2.5f : 1.0f);
        draw_icon(dl, ImVec2(p.x + tile / 2, p.y + 40), 46, d, it.id);
        const char *nm = d ? (ImGui::CalcTextSize(d->name.c_str()).x * 0.8f < tile - 6 ? d->name.c_str() : d->shortname.c_str()) : "?";
        ImGui::PushFont(ui_font_small);
        ImVec2 ts = ImGui::CalcTextSize(nm);
        dl->AddText(ImVec2(p.x + (tile - ts.x) / 2, p.y + tile - ts.y - 4), col(0.2f, 0.12f, 0.06f), nm);
        char q[16]; snprintf(q, sizeof q, "q%d", it.qual);
        dl->AddText(ImVec2(p.x + 5, p.y + 3), col(0.45f, 0.35f, 0.25f), q);
        if (it.count > 1) {
            char c[16]; snprintf(c, sizeof c, "x%d", it.count);
            ImVec2 cs = ImGui::CalcTextSize(c);
            dl->AddRectFilled(ImVec2(p.x + tile - cs.x - 10, p.y + 3), ImVec2(p.x + tile - 3, p.y + cs.y + 5), col(0.55f, 0.24f, 0.13f), 4);
            dl->AddText(ImVec2(p.x + tile - cs.x - 6, p.y + 4), col(1, 0.95f, 0.85f), c);
        }
        if (it.equipped) {
            const char *e = (d && (d->kind == K_ARMOR)) ? "worn" : "in hand";
            ImVec2 es = ImGui::CalcTextSize(e);
            dl->AddText(ImVec2(p.x + (tile - es.x) / 2, p.y + tile - 2 * es.y - 4), col(0.45f, 0.18f, 0.10f), e);
        }
        ImGui::PopFont();
        if (hov && !ImGui::IsMouseDragging(0) && !ImGui::IsPopupOpen("item_ctx")) tooltip(it, d);
        if (ImGui::BeginPopupContextItem("item_ctx")) {
            ImGui::TextDisabled("%s", d ? d->name.c_str() : "?");
            for (int m = 0; m < MAXM; m++) {
                if (m == sel_member || !member_present(m)) continue;
                if (ImGui::MenuItem(("Give to " + member_name(m, false)).c_str())) give(sel_member, m, it.idx);
                if (it.count > 1 && ImGui::MenuItem(("Give one to " + member_name(m, false)).c_str())) give(sel_member, m, it.idx, true);
            }
            if (equippable(d, it.type)) {
                ImGui::Separator();
                if (!it.equipped && ImGui::MenuItem((d && d->kind == K_ARMOR) ? "Wear" : "Take in hand")) equip(sel_member, it.idx, true);
                if (it.equipped && ImGui::MenuItem((d && d->kind == K_ARMOR) ? "Take off" : "Put away")) equip(sel_member, it.idx, false);
            }
            ImGui::EndPopup();
        }
        if (ImGui::BeginDragDropSource()) {
            DragItem di = { sel_member, it.idx };
            ImGui::SetDragDropPayload("DL_ITEM", &di, sizeof di);
            ImVec2 cp = ImGui::GetCursorScreenPos();
            ImGui::Dummy(ImVec2(52, 52));
            draw_icon(ImGui::GetWindowDrawList(), ImVec2(cp.x + 26, cp.y + 26), 44, d, it.id);
            ImGui::Text("%s", d ? d->name.c_str() : "?");
            ImGui::EndDragDropSource();
        }
        ImGui::PopID();
        col_i = (col_i + 1) % cols;
    }
    if (items.empty()) ImGui::TextDisabled("Carries nothing.");
    ImGui::EndChild();
    static std::string shown; static double shown_t;
    if (last_msg != shown) { shown = last_msg; shown_t = ImGui::GetTime(); }
    else if (!last_msg.empty() && ImGui::GetTime() - shown_t > 8) last_msg.clear();
    const char *lock = why_locked();
    ImGui::TextDisabled("%s", !last_msg.empty() ? last_msg.c_str() : lock ? lock
                        : "Drag an item onto a name to hand it over (hold Shift for just one); right-click for more.");
}

/* ---------------------------------------------------------------- the alchemy tab */
static int party_stock(int item, int *by_member)
{
    int total = 0;
    for (int m = 0; m < MAXM; m++) {
        if (by_member) by_member[m] = 0;
        if (!member_present(m)) continue;
        for (auto &it : member_items(m)) if (it.id == item) { total += it.count; if (by_member) by_member[m] += it.count; }
    }
    return total;
}

static bool knows(int m, int f)
{
    u16 o = (u16)(0x07be + m * 22 + f / 3);
    return (rb(G_FAR2E38, o) >> (f % 3)) & 1;
}

static std::string virtue_name(int f)
{
    u16 o = rw(G_FAR2E38, (u16)(0x1736 + f * 4)), s = rw(G_FAR2E38, (u16)(0x1736 + f * 4 + 2));
    char b[48]; if (!s) return "";
    g_str(s, o, b, sizeof b);
    return b;
}

extern "C" void tab_alchemy(void)
{
    if (!gamedata.loaded || gamedata.formulas.empty()) { ImGui::Text("The formula list (DARKLAND.ALC) could not be read."); return; }
    ImGui::BeginChild("alc", ImVec2(0, 0));
    // ingredient shelf
    ImGui::SeparatorText("The party's ingredients");
    ImDrawList *dl = ImGui::GetWindowDrawList();
    int shown = 0;
    for (int id = 72; id <= 94; id++) {
        int byM[MAXM]; int n = party_stock(id, byM);
        const ItemDef *d = def_of(id);
        if (shown % 8) ImGui::SameLine();
        ImGui::PushID(id);
        ImVec2 p = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton("ing", ImVec2(88, 92));
        dl->AddRectFilled(p, ImVec2(p.x + 88, p.y + 92), n ? col(0.96f, 0.92f, 0.82f) : col(0.88f, 0.83f, 0.72f, 0.6f), 6);
        dl->AddRect(p, ImVec2(p.x + 88, p.y + 92), col(0.55f, 0.42f, 0.25f), 6);
        draw_icon(dl, ImVec2(p.x + 44, p.y + 34), 40, d, id);
        ImGui::PushFont(ui_font_small);
        const char *sn = d ? d->shortname.c_str() : "?";
        ImVec2 ts = ImGui::CalcTextSize(sn);
        dl->AddText(ImVec2(p.x + (88 - ts.x) / 2, p.y + 92 - ts.y - 3), n ? col(0.2f, 0.12f, 0.06f) : col(0.5f, 0.42f, 0.32f), sn);
        char c[12]; snprintf(c, sizeof c, "%d", n);
        dl->AddText(ImVec2(p.x + 5, p.y + 3), n ? col(0.55f, 0.24f, 0.13f) : col(0.6f, 0.5f, 0.4f), c);
        ImGui::PopFont();
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("%s: %d", d ? d->name.c_str() : "?", n);
            for (int m = 0; m < MAXM; m++) if (byM[m]) ImGui::Text("  %s carries %d", member_name(m, false).c_str(), byM[m]);
            ImGui::EndTooltip();
        }
        ImGui::PopID();
        shown++;
    }
    // formulas
    static bool all_formulas = false;
    ImGui::SeparatorText(all_formulas ? "All formulas" : "Formulas the party knows");
    ImGui::Checkbox("Show all formulas (to judge one before buying it)", &all_formulas);
    if (ImGui::BeginTable("forms", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH)) {
        ImGui::TableSetupColumn("Potion", ImGuiTableColumnFlags_WidthFixed, 300);
        ImGui::TableSetupColumn("Known by", ImGuiTableColumnFlags_WidthFixed, 110);
        ImGui::TableSetupColumn("Ingredients", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Batches", ImGuiTableColumnFlags_WidthFixed, 70);
        ImGui::TableHeadersRow();
        int listed = 0;
        for (int f = 0; f < (int)gamedata.formulas.size(); f++) {
            std::string who;
            for (int m = 0; m < MAXM; m++) if (member_present(m) && knows(m, f)) { if (!who.empty()) who += ", "; who += member_name(m, false); }
            if (who.empty() && !all_formulas) continue;
            listed++;
            const Formula &F = gamedata.formulas[f];
            const ItemDef *pd = def_of(95 + f / 3);
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImVec2 p = ImGui::GetCursorScreenPos();
            ImGui::Dummy(ImVec2(28, 28));
            draw_icon(ImGui::GetWindowDrawList(), ImVec2(p.x + 14, p.y + 14), 26, pd, 95 + f / 3);
            ImGui::SameLine();
            { std::string vn = virtue_name(f); ImGui::TextWrapped("%s", vn.empty() ? (pd ? pd->name.c_str() : "?") : vn.c_str()); }
            if (ImGui::IsItemHovered()) { ImGui::BeginTooltip(); ImGui::PushTextWrapPos(360); ImGui::TextWrapped("%s", F.text.c_str()); ImGui::Text("Mystical number %d", F.mystic); ImGui::PopTextWrapPos(); ImGui::EndTooltip(); }
            if (all_formulas && f < (int)gamedata.formula_names.size()) {
                static const char *RISK[3] = { "low risk", "medium risk", "high risk" };
                ImGui::TextDisabled("%s - difficulty %d, %s", gamedata.formula_names[f].c_str(), F.mystic, RISK[F.risk >= 0 && F.risk < 3 ? F.risk : 0]);
            }
            ImGui::TableNextColumn(); ImGui::TextWrapped("%s", who.empty() ? "-" : who.c_str());
            ImGui::TableNextColumn();
            int batches = 999;
            for (size_t k = 0; k < F.ingr.size(); k++) {
                int have = party_stock(F.ingr[k].item, nullptr);
                const ItemDef *d = def_of(F.ingr[k].item);
                char lab[48]; snprintf(lab, sizeof lab, "%d %s", F.ingr[k].count, d ? d->shortname.c_str() : "?");
                float need = 24 + ImGui::CalcTextSize(lab).x + 10;
                if (k) { ImGui::SameLine(); if (ImGui::GetContentRegionAvail().x < need) ImGui::NewLine(); }
                ImVec2 q = ImGui::GetCursorScreenPos();
                ImGui::Dummy(ImVec2(22, 22));
                draw_icon(ImGui::GetWindowDrawList(), ImVec2(q.x + 11, q.y + 11), 20, d, F.ingr[k].item);
                ImGui::SameLine(0, 2);
                ImGui::TextColored(have >= F.ingr[k].count ? ImVec4(0.2f, 0.12f, 0.06f, 1) : ImVec4(0.65f, 0.15f, 0.10f, 1), "%s", lab);
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s: the party has %d", d ? d->name.c_str() : "?", have);
                int b = F.ingr[k].count ? have / F.ingr[k].count : 999;
                if (b < batches) batches = b;
            }
            ImGui::TableNextColumn();
            if (batches == 999) batches = 0;
            ImGui::TextColored(batches ? ImVec4(0.2f, 0.45f, 0.15f, 1) : ImVec4(0.6f, 0.5f, 0.4f, 1), "%d", batches);
        }
        ImGui::EndTable();
        if (!listed) ImGui::TextDisabled("Nobody in the party knows a formula.");
    }
    ImGui::EndChild();
}

extern "C" void ui_give(int from, int to, int idx) { give(from, to < 0 ? -to - 1 : to, idx, to < 0); fprintf(stderr, "ui: %s\n", last_msg.c_str()); }
