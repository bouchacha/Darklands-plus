#include "../mods/losaim.h"
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
/* over a member, while it reloads and aims a bow, crossbow, gun or the next thrown weapon, how far that has come
   (battle.reload_bar: cbt_missile_action, s014 0e76: action 9 reloading until D_dabd, 10 aiming
   until D_daa3, from D_d948 less the 10 ticks before reloading begins) */
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
    float age = (now - f.t0) / 1000.0f;                       /* 5 s: rises, then fades */
    if (age > 5.0f) return;
    float a = age < 0.15f ? age / 0.15f : age > 3.8f ? (5.0f - age) / 1.2f : 1.0f;
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
/* battle.los_line: the line of a shot about to be ordered and what stands across it (mods/losaim.c).
   The field's picture is the map sheared: a map point (x, y) is drawn at x - y across and y down,
   from the view's origin (D_c26c, D_c293: cbt_screen_pos, s014 14f7) */
static ImVec2 map2w(float x, float y, float up = 0)
{
    float ox = (s16)g_rw(0xc26c), oy = (s16)g_rw(0xc293);
    return g2w(60 + (x - ox) - (y - oy) + 0x88, (y - oy) + 0x69 - up);
}
static float seg_enter(float x0, float y0, float x1, float y1, const LosBox &b)   /* where the line meets the box, 0..1 (Liang-Barsky) */
{
    float t0 = 0, t1 = 1, dx = x1 - x0, dy = y1 - y0;
    float p[4] = { -dx, dx, -dy, dy }, q[4] = { x0 - b.x0, b.x1 - x0, y0 - b.y0, b.y1 - y0 };
    for (int k = 0; k < 4; k++) {
        if (p[k] == 0) { if (q[k] < 0) return 1; continue; }
        float r = q[k] / p[k];
        if (p[k] < 0) { if (r > t0) t0 = r; } else if (r < t1) t1 = r;
    }
    return t0 <= t1 ? t0 : 1;
}
static void draw_aim(ImDrawList *dl)
{
    int i, m, mode = orders_aim(&i, &m);
    if (!mode) return;
    LosBox bx[24];
    int n = los_trace(i, m, mode, bx, 24);
    float x0 = (s16)g_rw((u16)(GV_combatants + i * 0x80 + 4)), y0 = (s16)g_rw((u16)(GV_combatants + i * 0x80 + 6));
    float x1 = (s16)g_rw((u16)(GV_combatants + m * 0x80 + 4)), y1 = (s16)g_rw((u16)(GV_combatants + m * 0x80 + 6));
    float k = g2w(0, 1).y - g2w(0, 0).y, lw = fmaxf(1.5f, 0.55f * k);
    const ImU32 RED = IM_COL32(235, 60, 40, 255), RED_F = IM_COL32(235, 60, 40, 90), CLEAR = IM_COL32(250, 214, 110, 230), EDGE = IM_COL32(20, 10, 4, 170);
    float t = 1;
    for (int j = 0; j < n; j++) {                              /* each thing in the way: where it stands, outlined */
        const LosBox &b = bx[j];
        ImVec2 q[4] = { map2w(b.x0, b.y0), map2w(b.x1 + 1, b.y0), map2w(b.x1 + 1, b.y1 + 1), map2w(b.x0, b.y1 + 1) };
        dl->AddQuadFilled(q[0], q[1], q[2], q[3], RED_F);
        dl->AddPolyline(q, 4, EDGE, ImDrawFlags_Closed, lw + 2);
        dl->AddPolyline(q, 4, RED, ImDrawFlags_Closed, lw);
        float e = seg_enter(x0, y0, x1, y1, b);
        if (e < t) t = e;
    }
    ImVec2 a = map2w(x0, y0), z = map2w(x1, y1), h = map2w(x0 + (x1 - x0) * t, y0 + (y1 - y0) * t);
    dl->AddLine(a, h, EDGE, lw + 2);
    dl->AddLine(a, h, n ? IM_COL32(250, 214, 110, 200) : CLEAR, lw);
    if (n) {                                                  /* past the first thing in the way: dashes, red */
        float L = sqrtf((z.x - h.x) * (z.x - h.x) + (z.y - h.y) * (z.y - h.y)), d = 3.0f * k;
        for (float s0 = 0; s0 < L; s0 += 2 * d) {
            float s1 = fminf(L, s0 + d);
            ImVec2 p0(h.x + (z.x - h.x) * s0 / L, h.y + (z.y - h.y) * s0 / L), p1(h.x + (z.x - h.x) * s1 / L, h.y + (z.y - h.y) * s1 / L);
            dl->AddLine(p0, p1, EDGE, lw + 2); dl->AddLine(p0, p1, RED, lw);
        }
        float r = 1.6f * k;                                    /* a cross where it stops */
        dl->AddLine(ImVec2(h.x - r, h.y - r), ImVec2(h.x + r, h.y + r), RED, lw * 1.3f);
        dl->AddLine(ImVec2(h.x - r, h.y + r), ImVec2(h.x + r, h.y - r), RED, lw * 1.3f);
    }
    dl->AddCircleFilled(a, lw * 1.4f, n ? IM_COL32(250, 214, 110, 220) : CLEAR, 10);
    dl->AddCircle(z, 2.2f * k, n ? RED : CLEAR, 16, lw);
}

void ui_draw_battle_bars(void)
{
    if (game_mode() == GM_BATTLE && !ns_showing() && !config_get_int("original", 0)) {
        ImDrawList *dl0 = ovl_list();
        dl0->PushClipRect(ui_g2w(60, 0), ui_g2w(320, 200), true);
        draw_aim(dl0);
        dl0->PopClipRect();
    }
    bool reload = config_get_int("battle.reload_bar", 1) != 0;
    if (game_mode() != GM_BATTLE) floats.clear();
    bool foe_bars = config_get_int("battle.info", 1) != 0;   /* (the enemies' Endurance: who is near falling) */
    if ((!reload && !foe_bars && floats.empty()) || game_mode() != GM_BATTLE || config_get_int("original", 0) || ns_showing()) return;
    u32 nowms = plat_ticks_ms();
    while (!floats.empty() && nowms > floats.front().t0 + 5000) floats.erase(floats.begin());
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
        int st = g_rb((u16)(b + 0x14)), end = (s8)g_rb((u16)(b + 0x5d)), endm = (s8)g_rb((u16)(b + 0x64));
        if (st != 1 || endm <= 0) continue;
        int x0 = (s16)g_rw((u16)(0xd8f1 + 2 * i)), x1 = (s16)g_rw((u16)(0xd8d7 + 2 * i)), y0 = (s16)g_rw((u16)(0xd8bd + 2 * i));
        if (x1 <= x0 || x1 - x0 > 60) continue;
        float cx = (x0 + x1) * 0.5f + 60, w = 10.0f, gy = (float)y0 - 5.5f;       /* (the hot spot starts at the head: the bar clear above it) */
        if (gy < 1) gy = 1;
        ImVec2 a = g2w(cx - w * 0.5f, gy), c = g2w(cx + w * 0.5f, gy);
        float h = fmaxf(2.0f, 0.9f * k), bw = c.x - a.x, below = a.y;
        for (const Float &f : floats) if (f.m == m) draw_float(dl, f, cx, gy, k, nowms);
        if (foe_bars && m >= 5 && (end < endm || ImGui::GetIO().KeyAlt)) {   /* an enemy's Endurance left, once it is hurt (Alt: all) */
            float fe = end < 0 ? 0 : end > endm ? 1 : (float)end / endm;
            ImU32 col = fe <= 0.25f ? IM_COL32(240, 70, 50, 240) : IM_COL32(215, 95, 70, 235);
            dl->AddRectFilled(ImVec2(a.x - 1, a.y - 1), ImVec2(a.x + bw + 1, a.y + h + 1), IM_COL32(10, 6, 2, 200));
            if (fe > 0) dl->AddRectFilled(a, ImVec2(a.x + bw * fe, a.y + h), col);
            below = a.y + h + 1;
        }
        int act = g_rb((u16)(GV_cbt_action + m));
        if (reload && m < 5 && (act == 9 || act == 10)) {         /* reloading and aiming: a thin amber line filling up */
            u16 s0 = (u16)(g_rw((u16)(0xd948 + 2 * m)) - 10), e0 = g_rw((u16)(0xdaa3 + 2 * m));
            u16 span = (u16)(e0 - s0), done_t = (u16)(now - s0);
            float f = span ? fminf(1.0f, (float)done_t / span) : 1.0f;
            if (done_t > span) f = (s16)done_t < 0 ? 0.0f : 1.0f;
            float rh = fmaxf(2.0f, 0.7f * k);
            ImVec2 r0(a.x, below);
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

/* the first battle with the easier controls: what changed, once (battle.intro_seen) */
extern "C" void ui_alert(const char *title, const char *body, const char *foot);
static void battle_intro(void)
{
    if (config_get_int("battle.intro_seen", 0) || !config_get_int("battle.quick_orders", 1)) return;
    config_set_int("battle.intro_seen", 1); config_save();
    ui_alert("Battles need fewer clicks",
             "Click a member (on the field, or their box at the left), then an enemy to attack or a spot to walk to: that click gives the order. "
             "Space pauses and resumes the battle; while it is paused, the next member without an order is chosen for you.\n\n"
             "A member with a bow, crossbow, gun or thrown weapon shoots when you click an enemy and keeps shooting: at the next enemy when theirs falls, "
             "moving for a clear line when a wall, a tree or someone stands in the way, and drawing their melee weapon only when an enemy is beside them. "
             "The bow or sword on their picture at the left switches them to melee from the start.\n\n"
             "Against armor their weapon cannot pierce, members attack Vulnerable (aimed at the gaps in the armor) by themselves.\n\n"
             "Point at an order button in the right margin to see what it does.",
             "All this is Darklands+; each part can be switched off, for the game's own ways, in F12 > Settings > Battles. F11 lists the battle's keys and clicks.");
}

/* battle.foe_arrow: no enemy of the fight on the screen, but some on this floor - an arrow at the
   edge of the field points to the nearest (from the middle of the view), and a click on it shows
   it. Only the fight's own enemies (combatants 5..12 up, on the floor shown): no more than
   scrolling about would find. The view: the 140 cells the game draws, DS:c866/c886 its origin,
   offsets at DS:1fc6/2052; cells 32 x 16 map pixels, drawn as the level map draws them */
static float arrow_box[4]; static int arrow_foe = -1, arrow_frame = -10;
extern "C" void battlescroll_look(int i);
extern "C" void plat_game_to_win(float gx, float gy, float *wx, float *wy);
extern "C" int battleoverlay_arrow_click(int x, int y)
{
    if (ImGui::GetFrameCount() - arrow_frame > 2 || arrow_foe < 0) return 0;
    if (x < arrow_box[0] || x > arrow_box[2] || y < arrow_box[1] || y > arrow_box[3]) return 0;
    battlescroll_look(arrow_foe);
    return 1;
}
static void foe_arrow(void)
{
    if (!config_get_int("battle.foe_arrow", 1) || !g_rb(GV_real_combat)) return;
    int fl = (s8)g_rb(GV_floor_shown), n = (s8)g_rb(GV_hotspot_count);
    if (n < 0 || n > 40) return;
    for (int k = 0; k < n; k++) { int c = g_rb((u16)(GV_hotspot_cbt + k)); if (c >= 5 && c < 13 && g_rb((u16)(GV_combatants + 0x80 * c + 0x14)) == 1) { if (getenv("DL_ARROW_LOG")) fprintf(stderr, "arrow: %d in sight (of %d spots)\n", c, n); return; } }   /* (one in sight) */
    float vx = 0, vy = 0; int nv = 0;
    int ox = (s8)g_rb(0xc866), oy = (s8)g_rb(0xc886);
    for (int k = 0; k < 140; k++) { vx += ox + (s8)g_rb((u16)(0x1fc6 + k)) + 0.5f; vy += oy + (s8)g_rb((u16)(0x2052 + k)) + 0.5f; nv++; }
    vx /= nv; vy /= nv;
    int best = -1; float bd = 1e9f, bdx = 0, bdy = 0;
    for (int c = 5; c < 13; c++) {
        u16 b = (u16)(GV_combatants + 0x80 * c);
        if (g_rb((u16)(b + 0x14)) != 1 || (s8)g_rb((u16)(GV_cbt_floor + c)) != fl) continue;
        float X = (s16)g_rw((u16)(b + 4)) / 32.0f - vx, Y = (s16)g_rw((u16)(b + 6)) / 16.0f - vy;
        float d = X * X + Y * Y;
        if (d < bd) { bd = d; best = c; bdx = X * 1.0f + Y * -0.5f; bdy = Y * 0.6f; }
    }
    if (best < 0 || (bdx == 0 && bdy == 0)) return;
    float len = sqrtf(bdx * bdx + bdy * bdy); bdx /= len; bdy /= len;
    float x0, y0, x1, y1; plat_game_to_win(60, 0, &x0, &y0); plat_game_to_win(320, 200, &x1, &y1);
    float cxw = (x0 + x1) * 0.5f, cyw = (y0 + y1) * 0.5f, k = (y1 - y0) / 200.0f, in = 16 * k;
    float tx = bdx > 0 ? (x1 - in - cxw) / bdx : bdx < 0 ? (x0 + in - cxw) / bdx : 1e9f;
    float ty = bdy > 0 ? (y1 - in - cyw) / bdy : bdy < 0 ? (y0 + in - cyw) / bdy : 1e9f;
    float t = fminf(tx, ty);
    ImVec2 tip(cxw + bdx * t, cyw + bdy * t);
    ImDrawList *dl = ImGui::GetForegroundDrawList();
    float L = 13 * k, W = 7 * k, pulse = 0.75f + 0.25f * sinf((float)ImGui::GetTime() * 4);
    ImVec2 base(tip.x - bdx * L, tip.y - bdy * L), nrm(-bdy, bdx);
    ImVec2 a(base.x + nrm.x * W, base.y + nrm.y * W), b2(base.x - nrm.x * W, base.y - nrm.y * W);
    dl->AddTriangleFilled(tip, a, b2, IM_COL32(20, 10, 4, 220));
    ImVec2 tip2(tip.x - bdx * 2 * k, tip.y - bdy * 2 * k), a2(base.x + nrm.x * (W - 2 * k) - bdx * k, base.y + nrm.y * (W - 2 * k) - bdy * k), b3(base.x - nrm.x * (W - 2 * k) - bdx * k, base.y - nrm.y * (W - 2 * k) - bdy * k);
    dl->AddTriangleFilled(tip2, a2, b3, IM_COL32(230, 80, 50, (int)(255 * pulse)));
    ImFont *fo = ui_font_overlay ? ui_font_overlay : ImGui::GetFont();
    const char *lab = "Enemies";
    float fs = 6.5f * k; ImVec2 ts = fo->CalcTextSizeA(fs, FLT_MAX, 0, lab);
    ImVec2 lp(base.x - bdx * (ts.x * 0.5f + 4 * k) - ts.x * 0.5f, base.y - bdy * (ts.y * 0.5f + 4 * k) - ts.y * 0.5f);
    dl->AddRectFilled(ImVec2(lp.x - 3, lp.y - 1), ImVec2(lp.x + ts.x + 3, lp.y + ts.y + 1), IM_COL32(20, 10, 4, 190), 3);
    dl->AddText(fo, fs, lp, IM_COL32(250, 220, 190, 255), lab);
    arrow_box[0] = fminf(fminf(tip.x, a.x), fminf(b2.x, lp.x)) - 4; arrow_box[1] = fminf(fminf(tip.y, a.y), fminf(b2.y, lp.y)) - 4;
    arrow_box[2] = fmaxf(fmaxf(tip.x, a.x), fmaxf(b2.x, lp.x + ts.x)) + 4; arrow_box[3] = fmaxf(fmaxf(tip.y, a.y), fmaxf(b2.y, lp.y + ts.y)) + 4;
    arrow_foe = best; arrow_frame = ImGui::GetFrameCount();
    if (ImGui::GetIO().MousePos.x >= arrow_box[0] && ImGui::GetIO().MousePos.x <= arrow_box[2] && ImGui::GetIO().MousePos.y >= arrow_box[1] && ImGui::GetIO().MousePos.y <= arrow_box[3])
        ImGui::SetTooltip("The nearest enemy is this way: click to look");
}

void ui_draw_battle_info(void)
{
    if (game_mode() != GM_BATTLE || config_get_int("original", 0) || ns_showing()) return;
    battle_intro();
    foe_arrow();
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
        if (best < 5 || ui_battle_numbers) ImGui::Text("Endurance %d/%d, Strength %d/%d", end, endm, str, strm);
        if (wm >= 0 && wm <= 33) ImGui::Text("Melee rating %d, aim %d", def, atk);   /* (+0x20 both ways hand to hand, +0x21 shooting: mods/gamemodel.h) */
        else ImGui::Text("Melee rating %d", def);
        if (best >= 5 && st == 1 && g_rb((u16)(GV_cbt_ends_battle + best)) == 1)
            ImGui::TextColored(ImVec4(0.55f, 0.15f, 0.10f, 1), "The master of this place: once this enemy has fallen and the fight is over, the place is won.");
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
            static const char *HN[6] = { "no armor", "soft armor", "stiff armor", "scale armor", "mail", "plate" };
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
                        line += std::string(line.back() == ':' ? " " : "; ") + wn + (r == 0 ? " pierces" : r == 1 ? " does half damage" : " is deflected");
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

