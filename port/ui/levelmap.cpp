// levelmap.cpp - Tab in a battle or while exploring a castle, mine or dungeon (the tactical
// view, which shows only 14 x 10 cells around the party): a map of the level as far as the
// party has seen it, in the view's own slant, with the party, the enemies in sight, doors and
// furniture. The game waits while it is open; Tab or Esc closes it.
//
// The level is floor_img (far 20a5:0000, level%d.flr): long cells[40][40] (column-major,
// cell (x, y) at (x * 40 + y) * 4), a negative cell is solid rock; byte 0 high nibble: door art,
// byte 1 high nibble: furniture, byte 3 bits 4..5: the door's state. The view's centre cell is
// DS:c866 / c886 and its 140 cells lie at the offsets DS:1fc6[k] / 2052[k] (s017 1ef8:0000);
// floor_shown DS:11ca; combatants DS:9c55 + 0x80 * i (+4 x, +6 y map pixels, 32 x 16 a cell;
// +0x14 status), their floors cbt_floor DS:a6b4.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <string>
#include "imgui.h"
#include "ui.h"
#include "gamedata.h"
extern "C" {
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/config.h"
}
extern ImFont *ui_font_small;

#define NFLOORS 16
static unsigned char seen[NFLOORS][40][40];
static bool open_lm;
static int prev_mode = -1;

static inline s32 cell(int x, int y)
{
    u16 o = (u16)((x * 40 + y) * 4);
    return (s32)((u32)rw(G_FAR20A5, o) | (u32)rw(G_FAR20A5, (u16)(o + 2)) << 16);
}
static int floor_now(void) { return (s8)g_rb(0x11ca); }

extern "C" int levelmap_is_open(void) { return open_lm; }
extern "C" int levelmap_available(void) { return game_mode() == GM_BATTLE && config_get_int("assist.level_map", 1) && !config_get_int("original", 0); }
extern "C" void levelmap_toggle(void) { open_lm = !open_lm && levelmap_available(); }

/* every frame: the cells in the view count as seen; a new battle or dungeon starts afresh */
extern "C" void levelmap_frame(void)
{
    int gm = game_mode();
    if (gm != GM_BATTLE) { prev_mode = gm; open_lm = false; return; }
    if (prev_mode != GM_BATTLE) memset(seen, 0, sizeof seen);
    prev_mode = gm;
    int fl = floor_now();
    if (fl < 0 || fl >= NFLOORS) return;
    int cx = (s8)g_rb(0xc866), cy = (s8)g_rb(0xc886);
    for (int k = 0; k < 140; k++) {
        int x = cx + (s8)g_rb((u16)(0x1fc6 + k)), y = cy + (s8)g_rb((u16)(0x2052 + k));
        if (x >= 0 && x < 40 && y >= 0 && y < 40) seen[fl][x][y] = 1;
    }
}

static unsigned char view_now[40][40];
static void read_view(void)
{
    memset(view_now, 0, sizeof view_now);
    int cx = (s8)g_rb(0xc866), cy = (s8)g_rb(0xc886);
    for (int k = 0; k < 140; k++) {
        int x = cx + (s8)g_rb((u16)(0x1fc6 + k)), y = cy + (s8)g_rb((u16)(0x2052 + k));
        if (x >= 0 && x < 40 && y >= 0 && y < 40) view_now[x][y] = 1;
    }
}
static bool in_view_now(int x, int y) { return x >= 0 && x < 40 && y >= 0 && y < 40 && view_now[x][y]; }

extern "C" void levelmap_draw(void)
{
    ImGuiIO &io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Level map", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
                 ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBringToFrontOnFocus);
    ImGui::PopStyleVar();
    ImDrawList *dl = ImGui::GetWindowDrawList();
    ImVec2 o = ImGui::GetWindowPos(), sz = io.DisplaySize;
    dl->AddRectFilled(o, ImVec2(o.x + sz.x, o.y + sz.y), IM_COL32(14, 12, 10, 255));
    read_view();
    if (getenv("DL_LEVELMAP_DUMP")) {        /* debug: the floor as text (. floor, # hidden with floor art, space rock) */
        static int done; if (!done++) {
            for (int y = 0; y < 40; y++) { char line[41]; for (int x = 0; x < 40; x++) { s32 c = cell(x, y); u8 b = rb(G_FAR20A5, (u16)(0x1900 + x * 40 + y));
                line[x] = c >= 0 ? (b ? '.' : ',') : (b ? '#' : ((c & 0x7fffffff) ? '+' : ' ')); } line[40] = 0; fprintf(stderr, "lm %2d |%s|\n", y, line); } }
    }
    int fl = floor_now();
    static bool all = false;
    all = config_get_int("assist.level_map_all", 0) != 0;
    if (fl < 0 || fl >= NFLOORS) { ImGui::End(); return; }
    auto known = [&](int x, int y) { return x >= 0 && x < 40 && y >= 0 && y < 40 && (all || seen[fl][x][y]); };

    /* the view's slant (s014: screen x = map x - map y, screen y = map y; a cell is 32 x 16 map
       pixels, and the picture's pixels are 1.2 times as tall as wide): a cell's x edge runs
       across, its y edge down and to the left */
    const float SX = 1.0f, YX = -0.5f, YY = 0.6f;
    /* floor: a cell >= 0; with "the whole level" also the rooms not yet revealed (hidden cells,
       bit 31, that have floor art; solid rock has none) */
    auto hidden_floor = [&](int x, int y) { return all && cell(x, y) < 0 && rb(G_FAR20A5, (u16)(0x1900 + x * 40 + y)) != 0; };
    auto is_floor = [&](int x, int y) { return x >= 0 && x < 40 && y >= 0 && y < 40 && (cell(x, y) >= 0 || hidden_floor(x, y)); };
    float minx = 1e9f, maxx = -1e9f, miny = 1e9f, maxy = -1e9f; int any = 0;
    for (int x = 0; x < 40; x++) for (int y = 0; y < 40; y++) {
        if (!known(x, y) || !is_floor(x, y)) continue;
        any = 1;
        for (int c = 0; c < 4; c++) {
            float X = (float)(x + (c & 1)), Y = (float)(y + (c >> 1));
            float px = X * SX + Y * YX, py = Y * YY;
            if (px < minx) minx = px; if (px > maxx) maxx = px; if (py < miny) miny = py; if (py > maxy) maxy = py;
        }
    }
    float top = UIS(54), bottom = UIS(40);
    if (!any) { minx = 0; maxx = 40; miny = 0; maxy = 20; }
    float s = fminf((sz.x - 80) / (maxx - minx + 2), (sz.y - top - bottom) / (maxy - miny + 2));
    if (s > 60) s = 60;
    ImVec2 org(o.x + sz.x * 0.5f - (minx + maxx) * 0.5f * s, o.y + top + (sz.y - top - bottom) * 0.5f - (miny + maxy) * 0.5f * s);
    auto P = [&](float X, float Y) { return ImVec2(org.x + (X * SX + Y * YX) * s, org.y + Y * YY * s); };

    const ImU32 FLOOR = IM_COL32(120, 112, 98, 255), FLOOR_NOW = IM_COL32(150, 140, 120, 255), ROCK = IM_COL32(52, 46, 40, 255),
                EDGE = IM_COL32(30, 26, 22, 255), DOOR = IM_COL32(150, 96, 40, 255), FURN = IM_COL32(88, 66, 44, 255),
                STAIRS = IM_COL32(196, 170, 90, 255), HIDDEN = IM_COL32(86, 80, 96, 255);
    /* rock next to known floor, then the floor */
    for (int x = 0; x < 40; x++) for (int y = 0; y < 40; y++) {
        if (!known(x, y)) continue;
        ImVec2 a = P((float)x, (float)y), b = P((float)x + 1, (float)y), cc = P((float)x + 1, (float)y + 1), d = P((float)x, (float)y + 1);
        if (!is_floor(x, y)) {
            bool by_floor = false;
            for (int dx = -1; dx <= 1 && !by_floor; dx++) for (int dy = -1; dy <= 1; dy++) {
                int u = x + dx, v = y + dy;
                if (known(u, v) && is_floor(u, v)) { by_floor = true; break; }
            }
            if (by_floor) dl->AddQuadFilled(a, b, cc, d, ROCK);
            continue;
        }
        dl->AddQuadFilled(a, b, cc, d, hidden_floor(x, y) ? HIDDEN : in_view_now(x, y) ? FLOOR_NOW : FLOOR);
        dl->AddQuad(a, b, cc, d, EDGE, 1.0f);
    }
    /* furniture */
    for (int x = 0; x < 40; x++) for (int y = 0; y < 40; y++) {
        if (!known(x, y)) continue;
        s32 c = cell(x, y);
        if (c < 0) continue;
        ImVec2 m = P(x + 0.5f, y + 0.5f);
        int furn = (c >> 12) & 15;
        if (furn) dl->AddRectFilled(ImVec2(m.x - s * 0.18f, m.y - s * 0.1f), ImVec2(m.x + s * 0.18f, m.y + s * 0.1f), FURN);
    }
    /* doors: count 20a5:1f52, 14-byte records at 1f54 (int x, int y, w, h in map pixels;
       s014 19cd), open or shut in 20a5:33d3[k] */
    {
        int n = (s8)rb(G_FAR20A5, 0x1f52);
        if (n < 0 || n > 40) n = 0;
        for (int k = 0; k < n; k++) {
            u16 r0 = (u16)(0x1f54 + 14 * k);
            int x = (s16)rw(G_FAR20A5, r0), y = (s16)rw(G_FAR20A5, (u16)(r0 + 2)), w = rb(G_FAR20A5, (u16)(r0 + 4)), h = rb(G_FAR20A5, (u16)(r0 + 5));
            float X0 = x / 32.0f, Y0 = y / 16.0f, X1 = (x + w) / 32.0f, Y1 = (y + h) / 16.0f;
            if (X1 - X0 < 0.25f) { float m = (X0 + X1) * 0.5f; X0 = m - 0.125f; X1 = m + 0.125f; }
            if (Y1 - Y0 < 0.25f) { float m = (Y0 + Y1) * 0.5f; Y0 = m - 0.125f; Y1 = m + 0.125f; }
            if (!known((int)((X0 + X1) * 0.5f), (int)((Y0 + Y1) * 0.5f))) continue;
            bool open = rb(G_FAR20A5, (u16)(0x33d3 + k)) != 0;
            ImVec2 a = P(X0, Y0), b = P(X1, Y0), c2 = P(X1, Y1), d = P(X0, Y1);
            dl->AddQuadFilled(a, b, c2, d, open ? IM_COL32(110, 80, 50, 255) : DOOR);
            dl->AddQuad(a, b, c2, d, open ? IM_COL32(200, 180, 140, 255) : IM_COL32(40, 24, 10, 255), 1.5f);
        }
    }
    /* stairs, ladders and ways out: count 20a5:2fb6, 14-byte records at 2fb8 (int x, int y, w, h
       in map pixels, +6 the floor they lead to; s014 19cd:1db8, s023 take_exit) */
    {
        int n = (s8)rb(G_FAR20A5, 0x2fb6);
        if (n < 0 || n > 6) n = 0;
        float labels[8][4]; int nlabels = 0;
        for (int k = 0; k < n; k++) {
            u16 r0 = (u16)(0x2fb8 + 14 * k);
            int x = (s16)rw(G_FAR20A5, r0), y = (s16)rw(G_FAR20A5, (u16)(r0 + 2)), w = rb(G_FAR20A5, (u16)(r0 + 4)), h = rb(G_FAR20A5, (u16)(r0 + 5));
            int to = (s8)rb(G_FAR20A5, (u16)(r0 + 6));
            float X0 = x / 32.0f, Y0 = y / 16.0f, X1 = (x + w) / 32.0f, Y1 = (y + h) / 16.0f;
            if (X1 - X0 < 0.5f) X1 = X0 + 0.5f; if (Y1 - Y0 < 0.5f) Y1 = Y0 + 0.5f;
            bool k_seen = false;
            for (int cx = (int)X0; cx <= (int)X1 && !k_seen; cx++) for (int cy = (int)Y0; cy <= (int)Y1; cy++)
                if (known(cx, cy) && is_floor(cx, cy)) { k_seen = true; break; }     /* not behind a door still shut */
            if (!k_seen) continue;
            ImVec2 a = P(X0, Y0), b = P(X1, Y0), c2 = P(X1, Y1), d = P(X0, Y1);
            dl->AddQuadFilled(a, b, c2, d, STAIRS);
            dl->AddQuad(a, b, c2, d, IM_COL32(250, 240, 200, 255), 2.0f);
            char lab[24];
            if (to >= 0 && to < NFLOORS && to != fl) snprintf(lab, sizeof lab, "to floor %d", to + 1); else snprintf(lab, sizeof lab, "way out");
            ImVec2 m = P((X0 + X1) * 0.5f, (Y0 + Y1) * 0.5f);
            ImVec2 ts = (ui_font_small ? ui_font_small : ImGui::GetFont())->CalcTextSizeA(UIS(17), FLT_MAX, 0, lab);
            ImVec2 l0(m.x - ts.x * 0.5f - 3, m.y - s * 0.5f - ts.y - 2), l1(m.x + ts.x * 0.5f + 3, m.y - s * 0.5f);
            for (int tries = 0; tries < 6; tries++) {          /* up and down stairs share a spot: labels above each other */
                bool clash = false;
                for (int q = 0; q < nlabels; q++)
                    if (l0.x < labels[q][2] && l1.x > labels[q][0] && l0.y < labels[q][3] && l1.y > labels[q][1]) { clash = true; break; }
                if (!clash) break;
                l0.y -= ts.y + 4; l1.y -= ts.y + 4;
            }
            if (nlabels < 8) { labels[nlabels][0] = l0.x; labels[nlabels][1] = l0.y; labels[nlabels][2] = l1.x; labels[nlabels][3] = l1.y; nlabels++; }
            dl->AddRectFilled(l0, l1, IM_COL32(20, 16, 12, 200), 3);
            dl->AddText(ui_font_small ? ui_font_small : ImGui::GetFont(), UIS(17), ImVec2(l0.x + 3, l0.y + 1), IM_COL32(250, 236, 190, 255), lab);
        }
    }
    /* the party, and the enemies the party can see now */
    ImFont *f = ui_font_small ? ui_font_small : ImGui::GetFont();
    float r = fmaxf(5.0f, s * 0.32f);
    for (int i = 12; i >= 0; i--) {
        u16 b = (u16)(0x9c55 + 0x80 * i);
        int st = g_rb((u16)(b + 0x14));
        if (st != 1 && st != 2) continue;
        if ((s8)g_rb((u16)(0xa6b4 + i)) != fl) continue;
        float X = (s16)g_rw((u16)(b + 4)) / 32.0f, Y = (s16)g_rw((u16)(b + 6)) / 16.0f;
        if (i >= 5 && !in_view_now((int)X, (int)Y)) continue;
        ImVec2 m = P(X, Y);
        ImU32 col = i < 5 ? (st == 1 ? IM_COL32(70, 150, 230, 255) : IM_COL32(60, 90, 120, 255)) : (st == 1 ? IM_COL32(210, 60, 50, 255) : IM_COL32(110, 50, 45, 255));
        dl->AddCircleFilled(m, r, col);
        dl->AddCircle(m, r, IM_COL32(10, 8, 6, 255), 0, 1.5f);
        if (i < 5) {
            char nm[16]; g_str(G_DGROUP, (u16)(G_MEMBER(i) + G_M_SHORTNAME), nm, sizeof nm);
            char ini[3] = { nm[0] ? nm[0] : '?', nm[0] ? nm[1] : 0, 0 };     /* two letters: Gretch and Gunther */
            float fs = r * 1.1f; ImVec2 ts = f->CalcTextSizeA(fs, FLT_MAX, 0, ini);
            dl->AddText(f, fs, ImVec2(m.x - ts.x * 0.5f, m.y - ts.y * 0.5f), IM_COL32(255, 255, 255, 255), ini);
        }
    }
    /* title and legend */
    char t[160];
    snprintf(t, sizeof t, all ? "The whole level  (floor %d)" : "The level, as far as the party has seen it  (floor %d)", fl + 1);
    dl->AddText(f, UIS(22), ImVec2(o.x + 16, o.y + 14), IM_COL32(230, 214, 180, 255), t);
    float lx = o.x + 16, ly = o.y + sz.y - UIS(30);
    auto legend = [&](ImU32 c, const char *txt, bool circle) {
        if (circle) dl->AddCircleFilled(ImVec2(lx + UIS(7), ly + UIS(9)), UIS(7), c); else dl->AddRectFilled(ImVec2(lx, ly + UIS(3)), ImVec2(lx + UIS(16), ly + UIS(15)), c);
        dl->AddText(f, UIS(18), ImVec2(lx + UIS(22), ly), IM_COL32(220, 205, 170, 255), txt);
        lx += UIS(30) + f->CalcTextSizeA(UIS(18), FLT_MAX, 0, txt).x;
    };
    legend(IM_COL32(70, 150, 230, 255), "party", true);
    legend(IM_COL32(210, 60, 50, 255), "enemies in sight", true);
    legend(FLOOR_NOW, "in view now", false);
    legend(DOOR, "door", false);
    legend(FURN, "furniture", false);
    legend(STAIRS, "stairs, ways out", false);
    if (all) legend(HIDDEN, "not yet open", false);
    dl->AddText(f, UIS(18), ImVec2(lx + 10, ly), IM_COL32(170, 155, 125, 255), "Tab or Esc closes this. The game waits meanwhile.");
    ImGui::End();
}
