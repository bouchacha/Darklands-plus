// margins.cpp - on a wide screen the 4:3 picture leaves dark margins; they can show what the
// game otherwise keeps behind menus: the date and hour, where the party is, the purse, the
// tasks the party has taken on (with their deadlines), and on the other side the town the party
// is in, the blessings and potions working on the party, and who carries too much.
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include "imgui.h"
#include "gamedata.h"
#include "ui.h"
extern "C" {
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "../mods/travel.h"
#include "../mods/menuview.h"
void plat_game_to_win(float gx, float gy, float *wx, float *wy);
}
extern ImFont *ui_font_text;
extern ImFont *ui_font_overlay;          /* ui.cpp: the text over and beside the picture */
struct Eff { int m; std::string what; long left; };   /* (the same as ui/saints.cpp's, which makes them) */
std::vector<Eff> effects_now(int size);
void quest_brief(std::vector<std::pair<std::string, std::string>> &out);
extern "C" int citynav_available(void);
extern "C" int battlelog_margin(float x, float w, float h, float fs);

int ui_margins_on = 1;

static const char *MONTHS[12] = { "January", "February", "March", "April", "May", "June", "July", "August",
                                  "September", "October", "November", "December" };

static std::string place_name(void)
{
    int cur = (s16)g_rw(G_CUR_LOC);
    if (cur < 0) {                                   /* in the wilds: the nearest town or place */
        int px, py; travel_party_tile(&px, &py);
        int best = -1; long bd = 1L << 30;
        for (int k = 0; k < G_NLOC; k++) {
            u32 r = g_location(k);
            if (!r) continue;
            int x = (s16)rw((u16)(r >> 16), (u16)((u16)r + 4)), y = (s16)rw((u16)(r >> 16), (u16)((u16)r + 6));
            long dx = (x - px) * 4, dy = (y - py), d = dx * dx + dy * dy;      /* tiles are 16x4 apart */
            if (d < bd) { bd = d; best = k; }
        }
        if (best < 0 || bd > 60 * 60) return "In the wilds";
        std::string n = best < (int)gamedata.cities.size() ? gamedata.cities[best].name : "";
        if (n.empty()) { u32 r = g_location(best); char b[24]; g_str((u16)(r >> 16), (u16)((u16)r + 0x26), b, sizeof b); n = game_text(b, sizeof b); }
        return "Travelling, near " + n;
    }
    if (cur < (int)gamedata.cities.size()) return gamedata.cities[cur].name;
    u32 r = g_location(cur);
    if (!r) return "";
    char b[24]; g_str((u16)(r >> 16), (u16)((u16)r + 0x26), b, sizeof b);
    return game_text(b, sizeof b);
}

static std::string member_name(int m)
{
    char n[16]; g_str(G_DGROUP, (u16)(G_MEMBER(m) + G_M_SHORTNAME), n, sizeof n);
    return game_text(n, sizeof n);
}

static bool begin_panel(const char *id, float x, float w, float h, float size)
{
    ImGui::SetNextWindowPos(ImVec2(x, 0));
    ImGui::SetNextWindowSize(ImVec2(w, h));
    ImGui::SetNextWindowBgAlpha(0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(size * 0.7f, size * 0.8f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(size * 0.3f, size * 0.22f));
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(232, 218, 186, 255));
    ImGui::PushStyleColor(ImGuiCol_TextDisabled, IM_COL32(170, 150, 120, 255));
    ImGui::PushStyleColor(ImGuiCol_Separator, IM_COL32(120, 92, 56, 200));
    bool open = ImGui::Begin(id, nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoSavedSettings |
                                          ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNav);
    ImGui::PushFont(ui_font_overlay);
    ImGui::SetWindowFontScale(size / ui_font_overlay->FontSize);
    return open;
}
static void end_panel(void)
{
    ImGui::PopFont();
    ImGui::End();
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);
}
static void heading(const char *t)
{
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(222, 178, 104, 255));
    ImGui::TextUnformatted(t);
    ImGui::PopStyleColor();
}

/* in a battle: everybody's condition at a glance - the party with endurance and strength,
 * the enemies the party has seen (in the battle's hot spots) with how hurt they are */
std::string battlelog_name(int i);
static void bar(float frac, ImU32 col, float w, float hgt)
{
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList *dl = ImGui::GetWindowDrawList();
    if (frac < 0) frac = 0;
    if (frac > 1) frac = 1;
    dl->AddRectFilled(p, ImVec2(p.x + w, p.y + hgt), IM_COL32(60, 46, 30, 255), hgt * 0.4f);
    if (frac > 0) dl->AddRectFilled(p, ImVec2(p.x + w * frac, p.y + hgt), col, hgt * 0.4f);
    ImGui::Dummy(ImVec2(w, hgt));
}
extern "C" int partypanel_shown(void);
/* the orders for the member being given them: the game's Attack and Orders menus as buttons
   (only what the game allows now; chosen at the battle's next round, as from the menu). The
   margins take no input from ImGui: the buttons are drawn here and their clicks come through
   margins_click (ui_event) */
struct OrderBtn { ImVec2 a, b; int m, i, s; };
static std::vector<OrderBtn> order_btns; static int order_btns_frame = -100;
extern "C" int margins_click(int x, int y)
{
    if (ImGui::GetFrameCount() - order_btns_frame > 2) return 0;
    for (const OrderBtn &o : order_btns)
        if (x >= o.a.x && x < o.b.x && y >= o.a.y && y < o.b.y) { menuview_request(o.m, o.i, o.s); return 1; }
    return 0;
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
                if (e.key[0]) label += std::string(" (") + e.key + ")";
                ImVec2 ts = ImGui::CalcTextSize(label.c_str());
                float w = ts.x + pad * 2, h = lh + pad;
                if (x > x0 && x + w > x1) { x = x0; y += h + gap; }
                ImVec2 a(x, y), b(x + w, y + h);
                bool hov = mp.x >= a.x && mp.x < b.x && mp.y >= a.y && mp.y < b.y;
                dl->AddRectFilled(a, b, hov ? IM_COL32(150, 112, 56, 255) : IM_COL32(96, 70, 36, 255), h * 0.2f);
                dl->AddRect(a, b, IM_COL32(170, 132, 70, 255), h * 0.2f);
                dl->AddText(ImVec2(a.x + pad, a.y + pad * 0.5f), IM_COL32(244, 232, 204, 255), label.c_str());
                order_btns.push_back({ a, b, j, k, s2 });
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

static void battle_panel(float width, float fs)
{
    static bool seen[13]; static u16 last_tick = 0xffff;
    static int last_st[13]; static std::string last_name[13];      /* the fallen: the game clears their records */
    u16 tick = g_rw(0xe092);                        /* the battle's clock: back at 0 in a new battle */
    if (tick < last_tick) for (int i = 0; i < 13; i++) { seen[i] = false; last_st[i] = 0; last_name[i].clear(); }
    last_tick = tick;
    int n = (s8)g_rb(GV_hotspot_count);
    for (int k = 0; k < n && k < 40; k++) { int m = g_rb((u16)(GV_hotspot_cbt + k)); if (m < 13) seen[m] = true; }
    float bw = width - fs * 1.6f, bh = fs * 0.32f;
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
        if (!seen[i]) continue;
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
        }
        shown++;
    }
    if (!shown) ImGui::TextDisabled("None in sight yet.");
    else if (down == shown) ImGui::TextDisabled("All down.");
}

/* the text size for what the interface writes over and beside the picture (side panels,
 * tooltips, names on the map): grows with the picture, times display.text_size (%) */
extern "C" float ui_text_px(void)
{
    float x0, y0, x1, y1; plat_game_to_win(0, 0, &x0, &y0); plat_game_to_win(320, 200, &x1, &y1);
    float ph = y1 - y0; if (ph < 200) ph = ImGui::GetIO().DisplaySize.y;
    float k = config_get_int("display.text_size", 100) / 100.0f; if (k < 0.5f) k = 0.5f; if (k > 2.5f) k = 2.5f;
    float px = ph / 34.0f; if (px < 14) px = 14;
    return px * k;
}

/* the right margin, out of battles: what is not on the party's cards at the left - the town
 * the party is in (its standing there, the buildings and shops), the blessings and potions
 * working and how long they last, and who carries too much */
extern "C" int ui_nearest_town(void)                /* in the wilds: the nearest town, if one is near (-1 none) */
{
    int px, py; travel_party_tile(&px, &py);
    int best = -1; long bd = 1L << 30;
    for (int k = 0; k < (int)gamedata.cities.size() && k < G_NLOC; k++) {
        u32 r = g_location(k);
        if (!r) continue;
        int x = (s16)rw((u16)(r >> 16), (u16)((u16)r + 4)), y = (s16)rw((u16)(r >> 16), (u16)((u16)r + 6));
        long dx = (x - px) * 4, dy = (y - py), d = dx * dx + dy * dy;      /* tiles are 16x4 apart */
        if (d < bd) { bd = d; best = k; }
    }
    return bd <= 60 * 60 ? best : -1;
}

static void right_panel(int size)
{
    int cur = (s16)g_rw(G_CUR_LOC);
    bool any = false, near = false;
    if (cur < 0 && game_mode() == GM_MAP) { cur = ui_nearest_town(); near = true; }
    if (cur >= 0 && cur < (int)gamedata.cities.size()) {
        const City &c = gamedata.cities[cur];
        heading(((near ? "Near " : "") + c.name).c_str());
        /* the location table is read while a card or the map is up; battles and their loot
           reuse that memory, so meanwhile the last value read stands */
        static std::map<int, int> rep_seen; static int rep_game = -1;
        if (rep_game != (s16)g_rw(G_PARTY_SEED)) { rep_seen.clear(); rep_game = (s16)g_rw(G_PARTY_SEED); }
        int gm = game_mode();
        u32 lr = g_location(cur);
        if (lr && (gm == GM_CARD || gm == GM_MAP)) rep_seen[cur] = (s16)rw((u16)(lr >> 16), (u16)((u16)lr + 0x12));
        int fame = (s16)g_rw(GV_party_fame);
        auto rp = rep_seen.find(cur);
        if (rp != rep_seen.end()) ImGui::TextWrapped("Reputation here %d, fame %d", rp->second, fame);
        else ImGui::TextWrapped("Fame %d", fame);
        static const struct { int bit; const char *name; } B[] = { { 1, "town hall" }, { 2, "fortress" }, { 4, "cathedral" }, { 7, "monastery" },
            { 8, "university" }, { 5, "armoury" }, { 10, "mint" }, { 9, "pawnshop" }, { 11, "docks" }, { 6, "slums" } };
        std::string b;
        for (auto &x : B) if (c.buildings & (1 << x.bit)) { b += b.empty() ? "" : ", "; b += x.name; }
        if (!b.empty()) { b[0] = (char)toupper((unsigned char)b[0]); ImGui::TextWrapped("%s", b.c_str()); }
        bool shops = false;
        for (int i = 0; i < 9; i++) {
            if (c.shopq[i] <= 0) continue;
            /* the shop's rating, and what its weapons and armour come out at: (25 + rating) / 2
               (barter's quality rule, s032 1462:0efa; most things have a base quality of 25) */
            if (!shops) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
                ImGui::TextWrapped("Shops: rating (their wares' quality)");
                ImGui::PopStyleColor(); shops = true;
            }
            ImGui::TextUnformatted(SHOP_NAMES[i]); ImGui::SameLine(); ImGui::TextDisabled("%d (%dq)", c.shopq[i], (25 + c.shopq[i]) / 2);
        }
        static std::vector<std::pair<int, std::string>> marks; static int mf = -1000;
        if (ImGui::GetFrameCount() - mf > 30) { void quest_marks(std::vector<std::pair<int, std::string>> &); quest_marks(marks); mf = ImGui::GetFrameCount(); }
        for (auto &m : marks) if (m.first == cur) ImGui::TextColored(ImVec4(0.92f, 0.78f, 0.40f, 1), "%s", m.second.c_str());
        any = true;
    }
    else if (cur >= (int)gamedata.cities.size() && cur < G_NLOC && game_mode() == GM_CARD) {   /* (battles reuse the location table) */
        /* a castle, monastery, village ...: what it is, and the quests that point there */
        u32 r = g_location(cur);
        if (r) {
            char b[24]; g_str((u16)(r >> 16), (u16)((u16)r + 0x26), b, sizeof b);
            const char *loc_kind_label(int k);
            heading(game_text(b, sizeof b).c_str());
            ImGui::TextDisabled("%s", loc_kind_label((s16)rw((u16)(r >> 16), (u16)r)));
            static std::vector<std::pair<int, std::string>> marks; static int mf = -1000;
            if (ImGui::GetFrameCount() - mf > 30) { void quest_marks(std::vector<std::pair<int, std::string>> &); quest_marks(marks); mf = ImGui::GetFrameCount(); }
            for (auto &m : marks) if (m.first == cur) ImGui::TextColored(ImVec4(0.92f, 0.78f, 0.40f, 1), "%s", m.second.c_str());
            if ((s16)rw((u16)(r >> 16), (u16)((u16)r + 0x18)) > 0) ImGui::TextWrapped("The party has goods stored here (F12 > Journal > Stored).");
            any = true;
        }
    }
    std::vector<Eff> e = effects_now(size);
    if (!e.empty()) {
        if (any) ImGui::Separator();
        heading("In force");
        for (const Eff &x : e) {
            if (x.left < 48) ImGui::TextWrapped("%s: %s (%ld h)", member_name(x.m).c_str(), x.what.c_str(), x.left);
            else ImGui::TextWrapped("%s: %s (%ld d)", member_name(x.m).c_str(), x.what.c_str(), (x.left + 12) / 24);
        }
        any = true;
    }
    /* the game's own load classes (s029 0e76:013a): the capacity is the current endurance +
       strength; light up to half of it, heavy above it, overloaded above one and a half times */
    bool head = false;
    for (int m = 0; m < size; m++) {
        u16 b = G_MEMBER(m);
        int st = g_rb((u16)(b + G_M_STATUS));
        if (!st || st == 3) continue;
        int end = g_rb((u16)(b + G_M_ATTR_CUR)), str = g_rb((u16)(b + G_M_ATTR_CUR + 1));
        int load = g_rb((u16)(b + G_M_LOAD)), cap = end + str;
        if (load <= cap) continue;
        if (!head) { if (any) ImGui::Separator(); heading("Burdens"); head = true; }
        if (load * 2 > cap * 3) ImGui::TextColored(ImVec4(0.95f, 0.45f, 0.35f, 1), "%s: overloaded, %d/%d", member_name(m).c_str(), load, cap);
        else ImGui::TextColored(ImVec4(0.92f, 0.78f, 0.40f, 1), "%s: heavy, %d/%d", member_name(m).c_str(), load, cap);
    }
    if (!any && !head) { ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled)); ImGui::TextWrapped("No blessings or potions at work; nobody carries too much."); ImGui::PopStyleColor(); }
}

extern "C" void ui_margins(void)
{
    if (!ui_margins_on || game_mode() == GM_OTHER || !ui_font_overlay) return;
    int size = (s16)g_rw(G_PARTY_SIZE);
    if (size < 1 || size > 5) return;
    ImGuiIO &io = ImGui::GetIO();
    float x0, y0, x1, y1; plat_game_to_win(0, 0, &x0, &y0); plat_game_to_win(320, 200, &x1, &y1);
    float left = x0, right = io.DisplaySize.x - x1;
    if (left < 150 && right < 150) return;
    float h = io.DisplaySize.y;
    /* with the picture (the margins' width keeps lines from getting too short), and the setting */
    float k = config_get_int("display.text_size", 100) / 100.0f;
    float fs = std::max(std::min(std::min(left, right) / 9.5f * k, ui_text_px()), 14.0f);

    if (left >= 150 && !battlelog_margin(0, left, h, fs) && begin_panel("##margin_l", 0, left, h, fs)) {
        int hour = (s16)g_rw(G_CUR_HOUR), day = (s16)g_rw(G_CUR_DAY), mon = (s16)g_rw(G_CUR_MONTH), year = (s16)g_rw(G_CUR_YEAR);
        heading("Today");
        ImGui::TextWrapped("%d %s %d", day, MONTHS[(unsigned)mon % 12], year);
        ImGui::TextDisabled("%d:00, %s", hour, hour < 5 || hour >= 21 ? "night" : hour < 9 ? "morning" : hour < 17 ? "day" : "evening");
        ImGui::Separator();
        heading("Where");
        ImGui::TextWrapped("%s", place_name().c_str());
        if (citynav_available() && config_get_int("assist.city_goto", 1)) ImGui::TextDisabled("G: go to a place here");
        ImGui::Separator();
        heading("Purse");
        int f = (s16)g_rw(G_MONEY), g = (s16)g_rw(G_MONEY + 2), p = (s16)g_rw(G_MONEY + 4), bn = (s16)g_rw(0x9072);
        ImGui::TextWrapped("%d florins, %d groschen, %d pfennigs", f, g, p);
        if (bn > 0) ImGui::TextDisabled("%d florins in bank notes", bn);
        ImGui::Separator();
        static std::vector<std::pair<std::string, std::string>> q; static int qf = -1000;
        if (ImGui::GetFrameCount() - qf > 30) { quest_brief(q); qf = ImGui::GetFrameCount(); }
        heading("Tasks");
        if (q.empty()) ImGui::TextDisabled("None taken on.");
        for (size_t i = 0; i < q.size() && i < 6; i++) {
            ImGui::TextWrapped("%s", q[i].first.c_str());
            if (!q[i].second.empty()) ImGui::TextDisabled("%s", q[i].second.c_str());
        }
        if (q.size() > 6) ImGui::TextDisabled("... %d more (F12 > Journal)", (int)q.size() - 6);
        ImGui::Separator();
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
        ImGui::TextWrapped("The game's menus (save, load, difficulty, party): a right click or F10.");
        ImGui::PopStyleColor();
        end_panel();
    }

    if (right >= 150 && game_mode() == GM_BATTLE && !config_get_int("original", 0) && begin_panel("##margin_rb", x1, right, h, fs)) {
        battle_panel(right, fs);
        end_panel();
    }
    else if (right >= 150 && begin_panel("##margin_r", x1, right, h, fs)) {
        right_panel(size);
        end_panel();
    }
}
