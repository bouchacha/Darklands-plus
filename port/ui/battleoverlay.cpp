#include <stdlib.h>
// battleoverlay.cpp - drawn over a battle: a bar over each fighter (endurance, and strength once it is
// down by half), who the pointer is on (condition, attack, defence, weapon, and against the party's
// weapons), and the fast-forward badge (Z). overlay.cpp calls these within its layer.
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <string>
#include <vector>
#include "imgui.h"
#include "../mods/nscreen.h"
#include "gamedata.h"
#include "ui.h"
#include "overlay.h"
extern "C" {
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "../runtime/hle.h"
#include "../mods/assist.h"
#include "../mods/hdtext.h"
}
extern ImFont *ui_font_small, *ui_font_text;
extern ImFont *ui_font_overlay;
extern ImFont *ui_font_game[UI_GAME_FONTS];
extern int ui_game_font, ui_text_edge;
static ImDrawList *ovl_list(void) { return ui_overlay_list(); }
extern "C" void plat_field_slide(float *x, float *y);
static ImVec2 g2w(float x, float y) { float sx, sy; plat_field_slide(&sx, &sy); return ui_g2w(x + sx, y + sy); }   /* (with the field while it slides) */
extern "C" int ns_showing(void);
extern "C" void plat_mouse_game(int *x, int *y);

/* in a battle: who the pointer is on (the battle's own hot spots, s014 19cd), their condition,
 * attack, defence and weapon; enemies' names from the battle's level%d.enm */
int ui_battle_info = 1, ui_battle_numbers = 0;
struct EnmRec { std::string name; int endm, strm, def, atk; };
/* the enemy types of a battle level (level%d.enm, 0x122-byte records after a count: a combatant
 * record, then the name at +0x84); a combatant is told by its maxima, attack and defence */
std::string ui_enemy_name(int floor, u16 b);
static std::string enemy_name(int floor, u16 b) { return ui_enemy_name(floor, b); }
std::string ui_enemy_name(int floor, u16 b)
{
    static std::vector<EnmRec> recs[8]; static long sizes[8] = { -1, -1, -1, -1, -1, -1, -1, -1 }; static long long stamps[8];
    if (floor < 0 || floor > 7) return "";
    char dn[16]; snprintf(dn, sizeof dn, "level%d.enm", floor);
    char p[1300]; dos_host_path(dn, p, sizeof p);
    struct stat stt; long long mt = stat(p, &stt) == 0 ? (long long)stt.st_mtime : 0;
    FILE *f = fopen(p, "rb");
    if (!f) return "";
    fseek(f, 0, SEEK_END); long n = ftell(f);
    if (n != sizes[floor] || mt != stamps[floor]) {          /* every battle writes the file anew, often at the same size */
        sizes[floor] = n; stamps[floor] = mt; recs[floor].clear();
        for (int k = 0; k < 16; k++) {
            unsigned char r[0x9b];
            if (fseek(f, 1 + (long)k * 0x122, SEEK_SET) || fread(r, 1, sizeof r, f) != sizeof r) break;
            char nm[24]; memcpy(nm, r + 0x84, 0x17); nm[0x17] = 0;
            recs[floor].push_back(EnmRec{ game_text(nm, sizeof nm), (s8)r[0x64], (s8)r[0x65], r[0x20], r[0x21] });
        }
    }
    fclose(f);
    int endm = (s8)g_rb((u16)(b + 0x64)), strm = (s8)g_rb((u16)(b + 0x65)), def = g_rb((u16)(b + 0x20)), atk = g_rb((u16)(b + 0x21));
    for (const EnmRec &e : recs[floor]) if (e.endm == endm && e.strm == strm && e.def == def && e.atk == atk) return e.name;
    for (const EnmRec &e : recs[floor]) if (e.endm == endm && e.strm == strm) return e.name;
    return "";
}
/* a thin bar over each fighter on the field: the endurance left (the party's blue, the enemies'
   red), and a second, the strength, once it is down by half (battle.bars); under it, while a
   member reloads and aims a bow, crossbow, gun or the next thrown weapon, how far that has come
   (battle.reload_bar: cbt_missile_action, s014 0e76: action 9 reloading until D_dabd, 10 aiming
   until D_daa3, from D_d948 less the 10 ticks before reloading begins) */
int ui_battle_bars = 1;
/* a skill raised in the fight (battle.skill_gains): "+2 Edged" rising in gold over the member's
   head for a few seconds (battlelog.cpp hands it on from the game's side message) */
struct Float { int m; std::string t; u32 t0; bool good; };
static std::vector<Float> floats;
extern "C" u32 plat_ticks_ms(void);
void ui_battle_float(int m, const std::string &t, bool good)
{
    if (m < 0 || m >= 5 || !config_get_int("battle.skill_gains", 1) || config_get_int("original", 0)) return;
    int stack = 0; for (const Float &f : floats) if (f.m == m) stack++;
    floats.push_back({ m, t, plat_ticks_ms() + (u32)(stack * 450), good });   /* (several at once: one after the other) */
}
static void draw_float(ImDrawList *dl, const Float &f, float cx, float gy, float k, u32 now)
{
    if (now < f.t0) return;
    float age = (now - f.t0) / 1000.0f;                       /* 3.5 s: rises, then fades */
    if (age > 3.5f) return;
    float a = age < 0.15f ? age / 0.15f : age > 2.6f ? (3.5f - age) / 0.9f : 1.0f;
    float rise = 6.0f + 9.0f * fminf(age, 1.6f) / 1.6f;
    ImFont *fo = ui_font_overlay ? ui_font_overlay : ImGui::GetFont();
    float sz = 9.0f * k * (age < 0.25f ? 1.0f + 0.35f * (0.25f - age) / 0.25f : 1.0f);   /* (a little bigger as it appears) */
    ImVec2 ts = fo->CalcTextSizeA(sz, FLT_MAX, 0, f.t.c_str());
    ImVec2 p = g2w(cx, gy - rise);
    p.x -= ts.x * 0.5f; p.y -= ts.y;
    int A = (int)(255 * a);
    ImU32 ink = f.good ? IM_COL32(255, 220, 110, A) : IM_COL32(245, 120, 100, A), edge = IM_COL32(20, 10, 2, (int)(230 * a));
    for (int dx = -1; dx <= 1; dx++) for (int dy = -1; dy <= 1; dy++) if (dx || dy) dl->AddText(fo, sz, ImVec2(p.x + dx * 1.5f, p.y + dy * 1.5f), edge, f.t.c_str());
    dl->AddText(fo, sz, p, ink, f.t.c_str());
}
void ui_draw_battle_bars(void)
{
    bool reload = config_get_int("battle.reload_bar", 1) != 0;
    if (game_mode() != GM_BATTLE) floats.clear();
    if ((!ui_battle_bars && !reload && floats.empty()) || game_mode() != GM_BATTLE || config_get_int("original", 0) || ns_showing()) return;
    u32 nowms = plat_ticks_ms();
    while (!floats.empty() && nowms > floats.front().t0 + 3500) floats.erase(floats.begin());
    bool hurt_only = config_get_int("battle.bars_hurt", 1) && !ImGui::GetIO().KeyAlt;
    int n = (s8)g_rb(GV_hotspot_count);
    if (n <= 0 || n > 40) return;
    ImDrawList *dl = ovl_list();
    bool done[13] = {};
    float k = g2w(0, 1).y - g2w(0, 0).y;                      /* window pixels per game row */
    dl->PushClipRect(ui_g2w(60, 0), ui_g2w(320, 200), true);  /* (the field only: not over the margins or the panel) */
    u16 now = g_rw(GV_combat_clock);
    for (int i = 0; i < n; i++) {
        int m = g_rb((u16)(GV_hotspot_cbt + i));
        if (m >= 13 || done[m]) continue;
        done[m] = true;
        u16 b = (u16)(GV_combatants + m * 0x80);
        int st = g_rb((u16)(b + 0x14)), end = (s8)g_rb((u16)(b + 0x5d)), endm = (s8)g_rb((u16)(b + 0x64)), str = (s8)g_rb((u16)(b + 0x5e)), strm = (s8)g_rb((u16)(b + 0x65));
        if (st != 1 || endm <= 0) continue;
        int x0 = (s16)g_rw((u16)(0xd8f1 + 2 * i)), x1 = (s16)g_rw((u16)(0xd8d7 + 2 * i)), y0 = (s16)g_rw((u16)(0xd8bd + 2 * i));
        if (x1 <= x0 || x1 - x0 > 60) continue;
        float cx = (x0 + x1) * 0.5f + 60, w = 10.0f, gy = (float)y0 - 5.5f;       /* (the hot spot starts at the head: the bar clear above it) */
        if (gy < 1) gy = 1;
        ImVec2 a = g2w(cx - w * 0.5f, gy), c = g2w(cx + w * 0.5f, gy);
        float h = fmaxf(2.0f, 0.9f * k), bw = c.x - a.x, below = a.y;
        for (const Float &f : floats) if (f.m == m) draw_float(dl, f, cx, gy, k, nowms);
        bool whole = end >= endm && !(strm > 0 && str * 2 <= strm);
        if (ui_battle_bars && !(hurt_only && whole)) {         /* (unhurt: none, unless Alt is held) */
            float fe = end < 0 ? 0 : end > endm ? 1 : (float)end / endm;
            ImU32 col = m < 5 ? IM_COL32(110, 140, 240, 235) : (fe <= 0.25f ? IM_COL32(240, 70, 50, 240) : IM_COL32(215, 95, 70, 235));
            dl->AddRectFilled(ImVec2(a.x - 1, a.y - 1), ImVec2(a.x + bw + 1, a.y + h + 1), IM_COL32(10, 6, 2, 200));
            if (fe > 0) dl->AddRectFilled(a, ImVec2(a.x + bw * fe, a.y + h), col);
            below = a.y + h + 1;
            if (strm > 0 && str * 2 <= strm) {                     /* gravely wounded: the strength too */
                float fs = str < 0 ? 0 : (float)str / strm;
                ImVec2 a2(a.x, below);
                dl->AddRectFilled(ImVec2(a2.x - 1, a2.y), ImVec2(a2.x + bw + 1, a2.y + h * 0.7f + 1), IM_COL32(10, 6, 2, 200));
                if (fs > 0) dl->AddRectFilled(a2, ImVec2(a2.x + bw * fs, a2.y + h * 0.7f), IM_COL32(110, 200, 100, 235));
                below = a2.y + h * 0.7f + 1;
            }
        }
        int act = g_rb((u16)(GV_cbt_action + m));
        if (reload && m < 5 && (act == 9 || act == 10)) {         /* reloading and aiming: a thin amber line filling up */
            u16 s0 = (u16)(g_rw((u16)(0xd948 + 2 * m)) - 10), e0 = g_rw((u16)(0xdaa3 + 2 * m));
            u16 span = (u16)(e0 - s0), done_t = (u16)(now - s0);
            float f = span ? fminf(1.0f, (float)done_t / span) : 1.0f;
            if (done_t > span) f = (s16)done_t < 0 ? 0.0f : 1.0f;
            float rh = fmaxf(2.0f, 0.7f * k);
            ImVec2 r0(a.x, below + (ui_battle_bars ? 0.5f : 0));
            dl->AddRectFilled(ImVec2(r0.x - 1, r0.y - 0.5f), ImVec2(r0.x + bw + 1, r0.y + rh + 0.5f), IM_COL32(10, 6, 2, 170));
            dl->AddRectFilled(r0, ImVec2(r0.x + bw * f, r0.y + rh), act == 10 ? IM_COL32(250, 214, 110, 235) : IM_COL32(205, 150, 60, 220));
        }
    }
    dl->PopClipRect();
}

/* what a click there would do, while a member is given orders (battle.click_hint, mods/orders.c):
   "Click: attack", or why not ("No clear shot"), the first line of the info, or alone by the pointer */
extern "C" const char *orders_hint(void);
extern "C" int plat_pointer_outside(void);
static void hint_line(const char *h)
{
    bool no = strncmp(h, "Click", 5) != 0;
    ImGui::TextColored(no ? ImVec4(0.62f, 0.20f, 0.12f, 1) : ImVec4(0.18f, 0.40f, 0.16f, 1), "%s", h);
}
static void place_by_pointer(ImVec2 p)
{
    ImGuiIO &io = ImGui::GetIO();
    bool right = p.x > io.DisplaySize.x * 0.6f, below = p.y > io.DisplaySize.y * 0.55f;
    ImGui::SetNextWindowPos(ImVec2(p.x + (right ? -26.0f : 26.0f), p.y + (below ? -26.0f : 26.0f)), ImGuiCond_Always, ImVec2(right ? 1.0f : 0.0f, below ? 1.0f : 0.0f));
    ImGui::SetNextWindowBgAlpha(0.92f);
}
static const ImGuiWindowFlags INFO_FLAGS = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize |
                                           ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoSavedSettings;

void ui_draw_battle_info(void)
{
    if (game_mode() != GM_BATTLE || config_get_int("original", 0) || ns_showing()) return;
    int gx, gy; plat_mouse_game(&gx, &gy);
    { int cx, cy, cw, ch; if (hdtext_game_cursor(&cx, &cy, &cw, &ch)) { gx = cx; gy = cy; } }
    int mx = gx - 60, my = gy, n = (s8)g_rb(GV_hotspot_count), best = -1, by = -32768;
    const char *hint = mx >= 0 && mx < 0x104 && !plat_pointer_outside() ? orders_hint() : nullptr;
    if (hint && (!ui_battle_info || n <= 0 || n > 40)) n = 0;          /* (the hint alone) */
    else if (!ui_battle_info || mx < 0 || n <= 0 || n > 40) return;
    for (int k = n - 1; k >= 0; k--) {
        int x0 = (s16)g_rw((u16)(0xd8f1 + 2 * k)), x1 = (s16)g_rw((u16)(0xd8d7 + 2 * k)), y0 = (s16)g_rw((u16)(0xd8bd + 2 * k)), y1 = (s16)g_rw((u16)(0xd8a3 + 2 * k));
        int m = g_rb((u16)(GV_hotspot_cbt + k));
        if (m >= 13 || mx < x0 || mx > x1 || my < y0 || my > y1) continue;
        int cyy = (s16)g_rw((u16)(GV_combatants + m * 0x80 + 6));
        if (cyy > by) { by = cyy; best = m; }
    }
    if (best < 0) {
        if (!hint) return;
        place_by_pointer(g2w((float)gx, (float)gy));
        if (ImGui::Begin("##clickhint", nullptr, INFO_FLAGS)) {
            ImGui::PushFont(ui_font_overlay);
            ImGui::SetWindowFontScale(ui_text_px() * 0.85f / ui_font_overlay->FontSize);
            hint_line(hint);
            ImGui::PopFont();
        }
        ImGui::End();
        return;
    }
    u16 b = (u16)(0x9c55 + best * 0x80);
    int st = g_rb((u16)(b + 0x14)), end = (s8)g_rb((u16)(b + 0x5d)), endm = (s8)g_rb((u16)(b + 0x64)), str = (s8)g_rb((u16)(b + 0x5e)), strm = (s8)g_rb((u16)(b + 0x65));
    /* +0x22 the missile weapon's type (0xff none), +0x23 the melee weapon's */
    int atk = g_rb((u16)(b + 0x21)), def = g_rb((u16)(b + 0x20)), wm = (s8)g_rb((u16)(b + 0x22)), w = (s8)g_rb((u16)(b + 0x23));
    std::string name;
    if (best < 5) { char nm[26]; g_str(G_DGROUP, (u16)(b + 0x25), nm, sizeof nm); name = game_text(nm, sizeof nm); }
    else name = enemy_name(g_rb((u16)(GV_cbt_floor + best)), b);
    if (name.empty()) name = best < 5 ? "?" : "Enemy";
    const char *cond = st == 3 ? "dead" : st == 2 ? "unconscious" : endm <= 0 ? "" : end * 4 <= endm ? "badly hurt" : end * 2 <= endm ? "hurt" : end < endm ? "a little hurt" : "unhurt";
    if (st == 1 && strm > 0 && str * 2 <= strm) cond = "gravely wounded";
    place_by_pointer(g2w((float)gx, (float)gy));
    if (ImGui::Begin("##battleinfo", nullptr, INFO_FLAGS)) {
        ImGui::PushFont(ui_font_overlay);
        ImGui::SetWindowFontScale(ui_text_px() * 0.85f / ui_font_overlay->FontSize);
        if (hint) hint_line(hint);
        ImGui::TextColored(best < 5 ? ImVec4(0.18f, 0.32f, 0.55f, 1) : ImVec4(0.55f, 0.15f, 0.10f, 1), "%s", name.c_str());
        if (*cond) { ImGui::SameLine(); ImGui::TextDisabled("- %s", cond); }
        if (best < 5 || ui_battle_numbers) ImGui::Text("Endurance %d/%d, strength %d/%d", end, endm, str, strm);
        ImGui::Text("Attack %d, defence %d", atk, def);
        if (best >= 5 && st == 1 && g_rb((u16)(GV_cbt_ends_battle + best)) == 1)
            ImGui::TextColored(ImVec4(0.55f, 0.15f, 0.10f, 1), "The master of this place: once this one has fallen and the fight is over, the place is won.");
        auto wname = [](int t) -> std::string {
            if (t < 0 || t > 33) return "";
            for (const ItemDef &d : gamedata.items) if (d.type == t && (d.kind == K_WEAPON || d.kind == K_MISSILE)) return game_text(d.name.c_str(), d.name.size());
            return "";
        };
        {
            std::string a = wname(w), m = wname(wm);
            if (!a.empty() || !m.empty()) ImGui::TextDisabled("%s%s%s", a.c_str(), !a.empty() && !m.empty() ? ", " : "", m.c_str());
        }
        if (best >= 5 && st == 1) {
            /* its body armour against the party's weapons (the game's rule, as in the armour
               tooltips: penetration below the hardness deflects most of the blow, equal halves it,
               above pierces and wounds strength) */
            int at = g_rb((u16)(b + 0x4b)), h = at == 0xff ? 0 : rb(G_FAR20A5, (u16)(0x781e + at));
            static const char *HN[6] = { "no armour", "soft armour", "stiff armour", "scale armour", "mail", "plate" };
            if (h >= 0 && h < 6) {
                ImGui::Text("%s (hardness %d)", HN[h], h);
                int size = (s16)g_rw(G_PARTY_SIZE); if (size < 0 || size > 5) size = 0;
                for (int m = 0; m < size; m++) {
                    u16 mb = (u16)(0x9c55 + m * 0x80);
                    if (g_rb((u16)(mb + 0x14)) != 1) continue;
                    char nm[16]; g_str(G_DGROUP, (u16)(G_MEMBER(m) + G_M_SHORTNAME), nm, sizeof nm);
                    std::string line = game_text(nm, sizeof nm) + ":";
                    int worst = 3;                                   /* the colour: the best of the member's weapons */
                    for (int k = 0; k < 2; k++) {
                        int mw = (s8)g_rb((u16)(mb + (k ? 0x22 : 0x23)));
                        std::string wn = wname(mw);
                        if (wn.empty()) continue;
                        int pen = rb(G_FAR20A5, (u16)(0x76a8 + mw)) & 15, r = pen > h ? 0 : pen == h ? 1 : 2;
                        if (r < worst) worst = r;
                        line += std::string(line.back() == ':' ? " " : "; ") + wn + (r == 0 ? " pierces" : r == 1 ? " is halved" : " is deflected");
                    }
                    if (worst == 3) continue;
                    ImVec4 c = worst == 0 ? ImVec4(0.18f, 0.40f, 0.16f, 1) : worst == 1 ? ImVec4(0.6f, 0.42f, 0.08f, 1) : ImVec4(0.62f, 0.20f, 0.12f, 1);
                    ImGui::TextColored(c, "%s", line.c_str());
                }
            }
        }
        ImGui::PopFont();
    }
    ImGui::End();
}

/* character creation: what the step on screen asks for and what the numbers on the right mean,
 * in the empty column at the left of the creation screen (s037 18c8, state D_8c1e) */
/* fast forward in a battle (Z, mods/pathfix.c): a badge at the field's top right */
void ui_draw_battle_fast(void)
{
    if (!battle_fast || game_mode() != GM_BATTLE || ns_showing()) return;
    ImDrawList *dl = ovl_list();
    const char *t = "\xe2\x96\xb6\xe2\x96\xb6 fast forward  (Z)";
    ImFont *f = ImGui::GetFont(); float fs = ImGui::GetFontSize();
    ImVec2 ts = f->CalcTextSizeA(fs, 1e9f, 0, t), r = g2w(318, 2);
    if (!f->FindGlyphNoFallback(0x25b6)) { t = ">> fast forward  (Z)"; ts = f->CalcTextSizeA(fs, 1e9f, 0, t); }
    ImVec2 a(r.x - ts.x - fs * 0.8f, r.y), b(r.x, r.y + ts.y + fs * 0.35f);
    dl->AddRectFilled(a, b, IM_COL32(40, 24, 12, 210), fs * 0.25f);
    dl->AddText(f, fs, ImVec2(a.x + fs * 0.4f, a.y + fs * 0.17f), IM_COL32(250, 230, 180, 255), t);
}

