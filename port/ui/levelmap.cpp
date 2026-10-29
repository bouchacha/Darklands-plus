// levelmap.cpp - Tab in a battle or while exploring a castle, mine or dungeon (the tactical
// view, which shows only 14 x 10 cells around the party): a map of the level as far as the
// party has seen it, in the view's own slant, with the party, the enemies in sight, doors and
// furniture, and at its top right the floors in short (a square a floor, the stairs seen between
// them). The game waits while it is open; Tab or Esc closes it.
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
#include <vector>
#include "imgui.h"
#include "ui.h"
#include "gamedata.h"
#include "richtext.h"
extern "C" {
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "../runtime/hooks.h"
}
extern ImFont *ui_font_small;
extern "C" int levelwalk_x, levelwalk_y, levelwalk_floor;   /* mods/levelwalk.c */
extern "C" int levelwalk_goal_x, levelwalk_goal_y, levelwalk_goal_floor;   /* (the place a walk from the map is going to) */
extern "C" char dlp_data_dir[];
extern "C" void levelwalk_stairs(int s);

#define NFLOORS 16
static unsigned char seen[NFLOORS][40][40];
static bool open_lm;
/* the ways between floors, as each floor's records stood when the party was last on it (only the
   floor shown is in memory): map pixels x, y, w, h and the floor led to (20a5:2fb8, +6) */
struct Exit { s16 x, y; u8 w, h; s8 to; };
static Exit exits[NFLOORS][6]; static int nexits[NFLOORS];
#define GV_NFLOORS 0xd661               /* the battle's number of floors (s002 tac start, D_d661) */
/* What the party saw of a castle, mine or dungeon is kept for the game (party seed) in darklands_levels_<seed>.bin
   beside the journal (dlp_data_dir): loading a save made in it, or coming back to it, forgot it all (1.4-RC7).
   One record a floor: the place (G_CUR_LOC), its number of floors, the floor, a fingerprint of the floor's
   layout (its cells' art, 20a5:1900, which no door or reveal changes), the cells seen (a bit each) and its
   ways to the other floors. A floor whose fingerprint does not match its record (a street fight in a town:
   another map each time) starts afresh, and its record is made anew. Out of a town, castle, mine or
   dungeon (a fight in the wilds) nothing is kept. */
#pragma pack(push, 1)
struct LvRec { s16 loc; u8 nf, fl; u32 hash; u8 bits[200]; u8 ne; Exit ex[6]; };
#pragma pack(pop)
static std::vector<LvRec> recs;
static int recs_seed = -99999, lv_loc = -2, lv_nf = -1;
static bool recs_dirty, fl_checked[NFLOORS];
static double recs_saved;
static void lv_path(char *b, size_t n, int seed) { snprintf(b, n, "%s/darklands_levels_%u.bin", dlp_data_dir, (unsigned)(u16)seed); }
static void lv_load(int seed)
{
    recs.clear(); recs_seed = seed; recs_dirty = false;
    char p[1200]; lv_path(p, sizeof p, seed);
    FILE *f = fopen(p, "rb");
    if (!f) return;
    char magic[6] = ""; u32 n = 0;
    if (fread(magic, 1, 6, f) == 6 && !memcmp(magic, "DLLV1", 6) && fread(&n, 4, 1, f) == 1 && n < 4096)
        for (u32 k = 0; k < n; k++) { LvRec r; if (fread(&r, sizeof r, 1, f) != 1) break; if (r.fl < NFLOORS && r.ne <= 6) recs.push_back(r); }
    fclose(f);
}
static void lv_save(void)
{
    if (!recs_dirty || recs_seed == -99999) return;
    recs_dirty = false; recs_saved = ImGui::GetTime();
    char p[1200]; lv_path(p, sizeof p, recs_seed);
    FILE *f = fopen(p, "wb");
    if (!f) return;
    u32 n = (u32)recs.size();
    fwrite("DLLV1", 1, 6, f); fwrite(&n, 4, 1, f);
    if (n) fwrite(recs.data(), sizeof(LvRec), n, f);
    fclose(f);
}
static u32 floor_hash(void)                      /* (FNV-1a of the floor's cell art) */
{
    u32 h = 2166136261u;
    for (int k = 0; k < 1600; k++) { h ^= rb(G_FAR20A5, (u16)(0x1900 + k)); h *= 16777619u; }
    return h;
}
static LvRec *lv_find(int loc, int nf, int fl) { for (LvRec &r : recs) if (r.loc == loc && r.nf == nf && r.fl == fl) return &r; return nullptr; }
/* every frame in a battle: the records of this place taken up once (seen and ways OR'd in), the floor shown
   checked against its record, and what is seen now written back (the file at most every few seconds) */
static void lv_keep(int fl)
{
    int seed = (s16)g_rw(G_PARTY_SEED), loc = (s16)g_rw(G_CUR_LOC), nf = g_rb(GV_NFLOORS);
    if (seed != recs_seed) { lv_save(); lv_load(seed); lv_loc = -2; }
    if (loc < 0 || nf < 1 || nf > NFLOORS) return;
    if (loc != lv_loc || nf != lv_nf) {
        lv_loc = loc; lv_nf = nf;
        for (int f = 0; f < NFLOORS; f++) fl_checked[f] = false;
        for (const LvRec &r : recs) {
            if (r.loc != loc || r.nf != nf) continue;
            for (int c = 0; c < 1600; c++) if (r.bits[c >> 3] >> (c & 7) & 1) seen[r.fl][c / 40][c % 40] = 1;
            if (!nexits[r.fl]) { for (int k = 0; k < r.ne; k++) exits[r.fl][k] = r.ex[k]; nexits[r.fl] = r.ne; }
        }
    }
    LvRec *r = lv_find(loc, nf, fl);
    if (!fl_checked[fl]) {
        fl_checked[fl] = true;
        u32 h = floor_hash();
        if (r && r->hash != h) {                   /* (another map under the same name: what was taken up is not this one's) */
            memset(seen[fl], 0, sizeof seen[fl]);
            r->hash = h; memset(r->bits, 0, sizeof r->bits); r->ne = 0; recs_dirty = true;
        }
        if (!r) { LvRec n{}; n.loc = (s16)loc; n.nf = (u8)nf; n.fl = (u8)fl; n.hash = h; recs.push_back(n); r = &recs.back(); recs_dirty = true; }
    }
    if (!r) return;
    for (int c = 0; c < 1600; c++)
        if (seen[fl][c / 40][c % 40] && !(r->bits[c >> 3] >> (c & 7) & 1)) { r->bits[c >> 3] |= (u8)(1 << (c & 7)); recs_dirty = true; }
    if (r->ne != nexits[fl] || memcmp(r->ex, exits[fl], sizeof(Exit) * (size_t)nexits[fl])) { r->ne = (u8)nexits[fl]; memcpy(r->ex, exits[fl], sizeof(Exit) * (size_t)nexits[fl]); recs_dirty = true; }
    if (recs_dirty && ImGui::GetTime() - recs_saved > 4.0) lv_save();
}
/* what the party saw is put away when a battle's floors are made (a new battle, castle, mine or dungeon:
   s004 0e76:0000, from s002's tac start) or come out of a save (EXPLODE, s003 1016:0d07, the battle save's
   blocks), and the kept records of the place taken up again (lv_keep); the same dungeon taken up again
   after a card (a riddle, a talk) keeps it as it is */
static void forget(void) { lv_save(); memset(seen, 0, sizeof seen); memset(nexits, 0, sizeof nexits); lv_loc = -2; }
static void floors_made_hook(void) { forget(); hook_call_original(); }
static void save_read_hook(void) { forget(); hook_call_original(); }

static inline s32 cell(int x, int y)
{
    u16 o = (u16)((x * 40 + y) * 4);
    return (s32)((u32)rw(G_FAR20A5, o) | (u32)rw(G_FAR20A5, (u16)(o + 2)) << 16);
}
static int floor_now(void) { return (s8)g_rb(GV_floor_shown); }

extern "C" int levelmap_is_open(void) { return open_lm; }
extern "C" int levelmap_available(void) { return game_mode() == GM_BATTLE && config_get_int("assist.level_map", 1) && !config_get_int("original", 0); }
extern "C" void levelmap_toggle(void) { open_lm = !open_lm && levelmap_available(); }

/* every frame: the cells in the view count as seen, and the floor's ways to the others are noted */
extern "C" void levelmap_frame(void)
{
    static bool hooked;
    if (!hooked) { hooked = true; hook_install(4, 0x0e76, 0x0000, floors_made_hook); hook_install(3, 0x1016, 0x0d07, save_read_hook); }
    int gm = game_mode();
    if (gm != GM_BATTLE) { open_lm = false; lv_save(); return; }
    int fl = floor_now();
    if (fl < 0 || fl >= NFLOORS) return;
    int ne = (s8)rb(G_FAR20A5, 0x2fb6);
    if (ne < 0 || ne > 6) ne = 0;
    for (int k = 0; k < ne; k++) {
        u16 r0 = (u16)(0x2fb8 + 14 * k);
        exits[fl][k] = { (s16)rw(G_FAR20A5, r0), (s16)rw(G_FAR20A5, (u16)(r0 + 2)), rb(G_FAR20A5, (u16)(r0 + 4)), rb(G_FAR20A5, (u16)(r0 + 5)), (s8)rb(G_FAR20A5, (u16)(r0 + 6)) };
    }
    nexits[fl] = ne;
    int cx = (s8)g_rb(0xc866), cy = (s8)g_rb(0xc886);
    for (int k = 0; k < 140; k++) {
        int x = cx + (s8)g_rb((u16)(0x1fc6 + k)), y = cy + (s8)g_rb((u16)(0x2052 + k));
        if (x >= 0 && x < 40 && y >= 0 && y < 40) seen[fl][x][y] = 1;
    }
    if (!config_get_int("original", 0)) lv_keep(fl);
}

/* the floors of a castle, mine or dungeon in short, at the level map's top right: a square a floor, the
   floors the party has been on and those its stairs lead to (a "?" while nobody has been there), lines
   for the stairs seen, the party's floor outlined; a click on a floor its stairs on this floor lead to
   walks the party to them. Laid out by how many stairs each is from the first floor (the way in). */
template <class Known>
static ImVec2 floors_draw(ImDrawList *dl, ImVec2 o, ImVec2 sz, int fl, Known known, bool measure)   /* the panel's size (0, 0: none) */
{
    int nf = g_rb(GV_NFLOORS);
    if (nf < 2 || nf > NFLOORS || fl < 0 || fl >= nf) return ImVec2(0, 0);
    bool visited[NFLOORS] = {}, shown[NFLOORS] = {};
    bool edge[NFLOORS][NFLOORS] = {};
    int via[NFLOORS]; for (int t = 0; t < NFLOORS; t++) via[t] = -1;    /* a seen exit of this floor to floor t */
    for (int f = 0; f < nf; f++) {
        for (int x = 0; x < 40 && !visited[f]; x++) for (int y = 0; y < 40; y++) if (seen[f][x][y]) { visited[f] = true; break; }
    }
    visited[fl] = true;
    for (int f = 0; f < nf; f++) {
        if (!visited[f]) continue;
        shown[f] = true;
        for (int k = 0; k < nexits[f]; k++) {
            const Exit &e = exits[f][k];
            int t = e.to;
            if (t < 0 || t >= nf || t == f) continue;
            bool ks = false;                                   /* the stairs seen on that floor */
            for (int cx = e.x / 32; cx <= (e.x + e.w - 1) / 32 && !ks; cx++) for (int cy = e.y / 16; cy <= (e.y + e.h - 1) / 16; cy++)
                if (cx >= 0 && cx < 40 && cy >= 0 && cy < 40 && (f == fl ? known(cx, cy) : seen[f][cx][cy] != 0)) { ks = true; break; }
            if (!ks) continue;
            edge[f][t] = edge[t][f] = true; shown[t] = true;
            if (f == fl && via[t] < 0) via[t] = k;
        }
    }
    /* rows: stairs from floor 0 (or from the first floor shown), then the floor's number */
    int depth[NFLOORS]; for (int f = 0; f < NFLOORS; f++) depth[f] = -1;
    int q[NFLOORS], qh = 0, qt = 0;
    for (int f0 = 0; f0 < nf; f0++) {
        if (!shown[f0] || depth[f0] >= 0) continue;
        int base = 0; for (int f = 0; f < nf; f++) if (depth[f] + 1 > base) base = depth[f] + 1;
        depth[f0] = base; q[qt++] = f0;
        while (qh < qt) { int f = q[qh++]; for (int t = 0; t < nf; t++) if (edge[f][t] && depth[t] < 0) { depth[t] = depth[f] + 1; q[qt++] = t; } }
    }
    int rows = 0, col[NFLOORS], rowlen[NFLOORS] = {};
    for (int f = 0; f < nf; f++) if (shown[f]) { col[f] = rowlen[depth[f]]++; if (depth[f] + 1 > rows) rows = depth[f] + 1; }
    int maxw = 1; for (int r = 0; r < rows; r++) if (rowlen[r] > maxw) maxw = rowlen[r];
    ImFont *f = ui_font_small ? ui_font_small : ImGui::GetFont();
    const float LS = fmaxf(UIS(22), ui_text_px());           /* (the text: as large as the margins', at the least) */
    float qs = fmaxf(UIS(34), LS * 1.6f), gx = UIS(20), gy = UIS(24), pad = UIS(12), th = LS * 1.25f;
    float pw = maxw * qs + (maxw - 1) * gx + 2 * pad, ph = th + rows * qs + (rows - 1) * gy + 2 * pad;
    if (pw < UIS(120)) pw = UIS(120);
    if (measure) return ImVec2(pw, ph);
    ImVec2 p0(o.x + sz.x - pw - UIS(16), o.y + UIS(14)), p1(p0.x + pw, p0.y + ph);
    dl->AddRectFilled(p0, p1, IM_COL32(26, 22, 18, 235), UIS(6));
    dl->AddRect(p0, p1, IM_COL32(120, 100, 70, 255), UIS(6), 0, 1.5f);
    dl->AddText(f, LS, ImVec2(p0.x + pad, p0.y + pad * 0.6f), IM_COL32(230, 214, 180, 255), "Floors");
    auto centre = [&](int fi) {
        float rw = rowlen[depth[fi]] * qs + (rowlen[depth[fi]] - 1) * gx;
        return ImVec2(p0.x + (pw - rw) * 0.5f + col[fi] * (qs + gx) + qs * 0.5f, p0.y + pad + th + depth[fi] * (qs + gy) + qs * 0.5f);
    };
    for (int a = 0; a < nf; a++) for (int b = a + 1; b < nf; b++)
        if (edge[a][b]) dl->AddLine(centre(a), centre(b), IM_COL32(196, 170, 90, 255), fmaxf(1.5f, UIS(2.5f)));
    bool party_on[NFLOORS] = {};
    for (int i = 0; i < 5; i++) { int st = g_rb(CBF(i, CB_STATUS)), pf = (s8)g_rb((u16)(GV_cbt_floor + i)); if ((st == 1 || st == 2) && pf >= 0 && pf < NFLOORS) party_on[pf] = true; }
    ImGuiIO &io = ImGui::GetIO();
    int hover = -1; ImVec2 ha, hb;
    for (int fi = 0; fi < nf; fi++) {
        if (!shown[fi]) continue;
        ImVec2 c = centre(fi), a(c.x - qs * 0.5f, c.y - qs * 0.5f), b(c.x + qs * 0.5f, c.y + qs * 0.5f);
        bool here = fi == fl;
        dl->AddRectFilled(a, b, here ? IM_COL32(150, 140, 120, 255) : visited[fi] ? IM_COL32(110, 102, 88, 255) : IM_COL32(44, 38, 32, 255), UIS(3));
        dl->AddRect(a, b, here ? IM_COL32(250, 220, 120, 255) : visited[fi] ? IM_COL32(30, 26, 22, 255) : IM_COL32(120, 104, 80, 255), UIS(3), 0, here ? 2.5f : 1.2f);
        char num[8]; snprintf(num, sizeof num, visited[fi] ? "%d" : "?", fi + 1);
        ImVec2 ts = f->CalcTextSizeA(LS, FLT_MAX, 0, num);
        dl->AddText(f, LS, ImVec2(c.x - ts.x * 0.5f, c.y - ts.y * 0.5f), visited[fi] ? IM_COL32(20, 16, 12, 255) : IM_COL32(200, 180, 140, 255), num);
        if (party_on[fi]) {                                        /* (the party there: the leader's head at the corner) */
            int lead = (s8)g_rb(GV_marching_order); if (lead < 0 || lead > 4) lead = 0;
            float hs = qs * 0.55f;
            ui_member_head(dl, lead, ImVec2(b.x - hs / 1.2f * 0.6f, a.y - hs * 0.4f), hs);
        }
        if (io.MousePos.x >= a.x && io.MousePos.x < b.x && io.MousePos.y >= a.y && io.MousePos.y < b.y) { hover = fi; ha = a; hb = b; }
    }
    if (hover >= 0) {
        char tip[200], tos[80] = "";
        int n = 0;
        for (int t = 0; t < nf; t++) if (edge[hover][t]) { char w[12]; snprintf(w, sizeof w, "%s%d", n ? ", " : "", t + 1); strncat(tos, w, sizeof tos - strlen(tos) - 1); n++; }
        if (hover == fl) snprintf(tip, sizeof tip, "Floor %d: the party is here.%s%s%s", hover + 1, n ? (n == 1 ? " Stairs seen to floor " : " Stairs seen to floors ") : "", tos, n ? "." : "");
        else if (visited[hover]) snprintf(tip, sizeof tip, "Floor %d: the party has been here.%s%s%s", hover + 1, n ? (n == 1 ? " Stairs seen to floor " : " Stairs seen to floors ") : "", tos, n ? "." : "");
        else snprintf(tip, sizeof tip, "A floor nobody has been on yet: stairs from floor %s lead there.", tos);
        if (via[hover] >= 0 && hover != fl) strncat(tip, g_rb(GV_real_combat) ? "\nClick: the party walks to those stairs." : "\nClick: the party walks to those stairs and takes them.", sizeof tip - strlen(tip) - 1);
        ui_tip_beside(ha, hb);
        ImGui::SetTooltip("%s", tip);
        if (via[hover] >= 0 && hover != fl && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            levelwalk_stairs(via[hover]);                      /* (mods/levelwalk.c: taken on arrival, outside a fight) */
            open_lm = false;
        }
    }
    return ImVec2(pw, ph);
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
    const float TS = fmaxf(UIS(28), ui_text_px() * 1.3f), LS = fmaxf(UIS(22), ui_text_px());   /* (the title and the rest: as large as the margins' text, at the least) */
    float top = TS * 2.2f, bottom = LS * 3.6f;
    if (!any) { minx = 0; maxx = 40; miny = 0; maxy = 20; }
    ImVec2 fp = floors_draw(dl, o, sz, fl, known, true);     /* (the floors' panel at the top right: the map beside it) */
    float right = fp.x > 0 ? fp.x + UIS(24) : 0;
    float s = fminf((sz.x - 80 - right) / (maxx - minx + 2), (sz.y - top - bottom) / (maxy - miny + 2));
    if (s > 60) s = 60;
    ImVec2 org(o.x + (sz.x - right) * 0.5f - (minx + maxx) * 0.5f * s, o.y + top + (sz.y - top - bottom) * 0.5f - (miny + maxy) * 0.5f * s);
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
            ImVec2 ts = (ui_font_small ? ui_font_small : ImGui::GetFont())->CalcTextSizeA(LS, FLT_MAX, 0, lab);
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
            dl->AddText(ui_font_small ? ui_font_small : ImGui::GetFont(), LS, ImVec2(l0.x + 3, l0.y + 1), IM_COL32(250, 236, 190, 255), lab);
        }
    }
    /* the party, and the enemies the party can see now */
    ImFont *f = ui_font_small ? ui_font_small : ImGui::GetFont();
    float r = fmaxf(5.0f, s * 0.32f);
    auto shown = [&](int i, ImVec2 *at) {            /* combatant i on this map: active or down, on this floor, an enemy only in sight */
        u16 b = (u16)(0x9c55 + 0x80 * i);
        int st = g_rb((u16)(b + CB_STATUS));
        if (st != 1 && st != 2) return false;
        if ((s8)g_rb((u16)(GV_cbt_floor + i)) != fl) return false;
        float X = (s16)g_rw((u16)(b + CB_X)) / 32.0f, Y = (s16)g_rw((u16)(b + CB_Y)) / 16.0f;
        if (i >= 5 && !in_view_now((int)X, (int)Y)) return false;
        *at = P(X, Y);
        return true;
    };
    /* who is going for whom (combatant +0x4a): the party's in blue, the enemies' in red */
    for (int i = 0; i < 13; i++) {
        ImVec2 a, b;
        int t = (s8)g_rb(CBF(i, CB_TARGET));
        if (t < 0 || t > 12 || t == i || !shown(i, &a) || !shown(t, &b)) continue;
        if (g_rb(CBF(i, CB_STATUS)) != 1) continue;
        ImU32 lc = i < 5 ? IM_COL32(90, 170, 250, 220) : IM_COL32(230, 80, 60, 220);
        float dx = b.x - a.x, dy = b.y - a.y, len = sqrtf(dx * dx + dy * dy);
        if (len < r * 2.2f) continue;
        dx /= len; dy /= len;
        ImVec2 a2(a.x + dx * r, a.y + dy * r), b2(b.x - dx * r * 1.1f, b.y - dy * r * 1.1f);
        dl->AddLine(a2, b2, lc, fmaxf(1.5f, s * 0.06f));
        float h = fmaxf(5.0f, s * 0.2f);
        dl->AddTriangleFilled(b2, ImVec2(b2.x - dx * h - dy * h * 0.5f, b2.y - dy * h + dx * h * 0.5f), ImVec2(b2.x - dx * h + dy * h * 0.5f, b2.y - dy * h - dx * h * 0.5f), lc);
    }
    for (int i = 12; i >= 0; i--) {
        u16 b = (u16)(0x9c55 + 0x80 * i);
        int st = g_rb((u16)(b + CB_STATUS));
        if (st != 1 && st != 2) continue;
        if ((s8)g_rb((u16)(GV_cbt_floor + i)) != fl) continue;
        float X = (s16)g_rw((u16)(b + CB_X)) / 32.0f, Y = (s16)g_rw((u16)(b + CB_Y)) / 16.0f;
        if (i >= 5 && !in_view_now((int)X, (int)Y)) continue;
        ImVec2 m = P(X, Y);
        if (i < 5) {                                           /* a member: their head, as on their box, in a blue ring (dimmed when down) */
            float rh = fmaxf(r * 1.35f, LS * 0.8f), hs = rh * 1.7f;
            dl->AddCircleFilled(m, rh, IM_COL32(18, 14, 10, 235));
            dl->AddCircle(m, rh, st == 1 ? IM_COL32(70, 150, 230, 255) : IM_COL32(60, 90, 120, 255), 0, fmaxf(2.0f, rh * 0.14f));
            ui_member_head(dl, i, ImVec2(m.x - hs / 1.2f * 0.5f, m.y - hs * 0.5f), hs, st == 1 ? IM_COL32_WHITE : IM_COL32(120, 120, 120, 255));
            continue;
        }
        ImU32 col = st == 1 ? IM_COL32(210, 60, 50, 255) : IM_COL32(110, 50, 45, 255);
        dl->AddCircleFilled(m, r, col);
        dl->AddCircle(m, r, IM_COL32(10, 8, 6, 255), 0, 1.5f);
    }
    /* a click on a floor cell, a door or stairs the party has seen: the party walks there
       (mods/levelwalk.c) */
    bool walk = true;                                   /* (always with the map; assist.level_map_walk retired) */
    bool on_floors = fp.x > 0 && io.MousePos.x >= o.x + sz.x - fp.x - UIS(16) && io.MousePos.y < o.y + UIS(14) + fp.y;
    if (walk && ImGui::IsWindowHovered() && !on_floors) {
        ImVec2 mp = io.MousePos;
        float Yf = (mp.y - org.y) / s / YY, Xf = ((mp.x - org.x) / s - Yf * YX) / SX;
        int hx = (int)floorf(Xf), hy = (int)floorf(Yf);
        int wx = -1, wy = -1;                                 /* where to, map pixels */
        float qx0 = 0, qy0 = 0, qx1 = 0, qy1 = 0;
        auto in_rect = [&](u16 r0, float grow) {               /* a door's or stairs' record: map pixels x, y, w, h */
            int x = (s16)rw(G_FAR20A5, r0), y = (s16)rw(G_FAR20A5, (u16)(r0 + 2)), w = rb(G_FAR20A5, (u16)(r0 + 4)), h = rb(G_FAR20A5, (u16)(r0 + 5));
            float X0 = x / 32.0f - grow, Y0 = y / 16.0f - grow, X1 = (x + w) / 32.0f + grow, Y1 = (y + h) / 16.0f + grow;
            if (Xf < X0 || Xf > X1 || Yf < Y0 || Yf > Y1 || !known((int)((X0 + X1) * 0.5f), (int)((Y0 + Y1) * 0.5f))) return false;
            wx = x + w / 2; wy = y + h / 2; qx0 = X0; qy0 = Y0; qx1 = X1; qy1 = Y1;
            return true;
        };
        int nd = (s8)rb(G_FAR20A5, 0x1f52), ns = (s8)rb(G_FAR20A5, 0x2fb6);
        for (int k = 0; k < ns && k < 6 && wx < 0; k++) in_rect((u16)(0x2fb8 + 14 * k), 0.15f);
        for (int k = 0; k < nd && k < 40 && wx < 0; k++)
            if (in_rect((u16)(0x1f54 + 14 * k), 0.3f)) {      /* a door stands in the wall: the floor cell before it, on the seen side */
                float bd = 1e9f; int bx = -1, by = -1;
                for (int x = (int)(wx / 32.0f) - 3; x <= (int)(wx / 32.0f) + 3; x++)
                    for (int y = (int)(wy / 16.0f) - 3; y <= (int)(wy / 16.0f) + 3; y++) {
                        if (x < 0 || x >= 40 || y < 0 || y >= 40 || !known(x, y) || cell(x, y) < 0) continue;
                        float dx = x + 0.5f - wx / 32.0f, dy = y + 0.5f - wy / 16.0f, d = dx * dx + dy * dy;
                        if (d < bd) { bd = d; bx = x; by = y; }
                    }
                if (bx < 0) { wx = -1; continue; }
                wx = bx * 32 + 16; wy = by * 16 + 8;
            }
        if (wx < 0 && known(hx, hy) && hx >= 0 && hx < 40 && hy >= 0 && hy < 40 && cell(hx, hy) >= 0) {
            wx = hx * 32 + 16; wy = hy * 16 + 8; qx0 = (float)hx; qy0 = (float)hy; qx1 = qx0 + 1; qy1 = qy0 + 1;
        }
        if (wx >= 0) {
            ImVec2 a = P(qx0, qy0), b = P(qx1, qy0), cc = P(qx1, qy1), d = P(qx0, qy1);
            dl->AddQuad(a, b, cc, d, IM_COL32(250, 220, 120, 255), 2.5f);
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                levelwalk_x = wx; levelwalk_y = wy; levelwalk_floor = fl;
                open_lm = false;
            }
        }
    }
    if (levelwalk_goal_floor == fl && levelwalk_goal_x >= 0) {   /* where a walk from the map is going: a gold ring */
        ImVec2 g = P(levelwalk_goal_x / 32.0f, levelwalk_goal_y / 16.0f);
        float rg = fmaxf(6.0f, s * 0.36f);
        dl->AddCircle(g, rg, IM_COL32(20, 14, 6, 255), 0, fmaxf(3.0f, s * 0.12f));
        dl->AddCircle(g, rg, IM_COL32(250, 210, 90, 255), 0, fmaxf(2.0f, s * 0.07f));
        dl->AddCircleFilled(g, rg * 0.25f, IM_COL32(250, 210, 90, 255));
    }
    floors_draw(dl, o, sz, fl, known, false);
    /* title and legend */
    char t[160];
    snprintf(t, sizeof t, all ? "The whole level  (floor %d)" : "The level, as far as the party has seen it  (floor %d)", fl + 1);
    dl->AddText(f, TS, ImVec2(o.x + 16, o.y + 14), IM_COL32(230, 214, 180, 255), t);
    float lx = o.x + 16, ly = o.y + sz.y - LS * 1.5f;
    auto legend = [&](ImU32 c, const char *txt, int circle) {
        float u = LS / 18.0f;
        if (circle == 2) { dl->AddCircleFilled(ImVec2(lx + 7 * u, ly + 9 * u), 7 * u, IM_COL32(18, 14, 10, 255)); dl->AddCircle(ImVec2(lx + 7 * u, ly + 9 * u), 7 * u, c, 0, 2.0f); }
        else if (circle) dl->AddCircleFilled(ImVec2(lx + 7 * u, ly + 9 * u), 7 * u, c); else dl->AddRectFilled(ImVec2(lx, ly + 3 * u), ImVec2(lx + 16 * u, ly + 15 * u), c);
        dl->AddText(f, LS, ImVec2(lx + 22 * u, ly), IM_COL32(220, 205, 170, 255), txt);
        lx += 30 * u + f->CalcTextSizeA(LS, FLT_MAX, 0, txt).x;
    };
    legend(IM_COL32(70, 150, 230, 255), "party", 2);
    legend(IM_COL32(210, 60, 50, 255), "enemies seen", true);
    legend(FLOOR_NOW, "in view now", false);
    legend(IM_COL32(90, 170, 250, 220), "arrows: who targets whom", false);
    legend(DOOR, "door", false);
    legend(FURN, "furniture", false);
    legend(STAIRS, "stairs, ways out", false);
    if (all) legend(HIDDEN, "not yet revealed", false);
    {   /* (what a click does, on a line of its own above the legend when the legend leaves no room) */
        const char *how = walk ? "Click the floor to walk the party there (the map closes, and they go). Tab or Esc closes the level map; the game is paused until then." : "Tab or Esc closes the level map; the game is paused until then.";
        float hw = f->CalcTextSizeA(LS, FLT_MAX, 0, how).x;
        if (lx + 10 + hw > o.x + sz.x - 16) { lx = o.x + 16 - 10; ly -= LS * 1.4f; }
        dl->AddText(f, LS, ImVec2(lx + 10, ly), IM_COL32(170, 155, 125, 255), how);
    }
    ImGui::End();
}

/* test (ufloor): the floor shown, as text in the log - the cells ('#' rock, '.' floor, 'P' a member), the doors
   (x, y, w, h map pixels, open), the ways out (and the floor they lead to) and the trigger areas (20a5:2184 count,
   14-byte records at 2186: int x, y, byte w, h (map pixels), the routine's far address, int a, b: its arguments) */
extern "C" void levelmap_dump(void)
{
    int fl = floor_now();
    fprintf(stderr, "floor %d\n", fl);
    char rows[40][41];
    for (int y = 0; y < 40; y++) { for (int x = 0; x < 40; x++) rows[y][x] = cell(x, y) < 0 ? '#' : '.'; rows[y][40] = 0; }
    for (int i = 0; i < 5; i++) {
        u16 b = (u16)(0x9c55 + i * 0x80);
        if (g_rb((u16)(b + 0x14)) == 0 || (s8)g_rb((u16)(GV_cbt_floor + i)) != fl) continue;
        int x = (s16)g_rw((u16)(b + 4)) / 32, y = (s16)g_rw((u16)(b + 6)) / 16;
        if (x >= 0 && x < 40 && y >= 0 && y < 40) rows[y][x] = (char)('0' + i);
    }
    for (int y = 0; y < 40; y++) fprintf(stderr, "  %2d %s\n", y, rows[y]);
    int nd = (s8)rb(G_FAR20A5, 0x1f52);
    for (int k = 0; k < nd && k < 40; k++) {
        u16 r0 = (u16)(0x1f54 + 14 * k);
        fprintf(stderr, "  door %d at %d,%d %dx%d open %d (cell %d,%d)\n", k, (s16)rw(G_FAR20A5, r0), (s16)rw(G_FAR20A5, (u16)(r0 + 2)), rb(G_FAR20A5, (u16)(r0 + 4)), rb(G_FAR20A5, (u16)(r0 + 5)),
                rb(G_FAR20A5, (u16)(0x33d3 + k)), (s16)rw(G_FAR20A5, r0) / 32, (s16)rw(G_FAR20A5, (u16)(r0 + 2)) / 16);
    }
    int ne = (s8)rb(G_FAR20A5, 0x2fb6);
    for (int k = 0; k < ne && k < 6; k++) {
        u16 r0 = (u16)(0x2fb8 + 14 * k);
        fprintf(stderr, "  exit %d at %d,%d %dx%d to %d (cell %d,%d)\n", k, (s16)rw(G_FAR20A5, r0), (s16)rw(G_FAR20A5, (u16)(r0 + 2)), rb(G_FAR20A5, (u16)(r0 + 4)), rb(G_FAR20A5, (u16)(r0 + 5)),
                (s8)rb(G_FAR20A5, (u16)(r0 + 6)), (s16)rw(G_FAR20A5, r0) / 32, (s16)rw(G_FAR20A5, (u16)(r0 + 2)) / 16);
    }
    int nh = (s8)rb(G_FAR20A5, 0x2184);
    for (int k = 0; k < nh && k < 50; k++) {
        u16 r0 = (u16)(0x2186 + 14 * k);
        s16 x = (s16)rw(G_FAR20A5, r0), y = (s16)rw(G_FAR20A5, (u16)(r0 + 2));
        fprintf(stderr, "  handler %d fn %04x:%04x a %d b %d at %d,%d %dx%d (cell %d,%d)\n", k, rw(G_FAR20A5, (u16)(r0 + 8)), rw(G_FAR20A5, (u16)(r0 + 6)),
                (s16)rw(G_FAR20A5, (u16)(r0 + 10)), (s16)rw(G_FAR20A5, (u16)(r0 + 12)), x, y, rb(G_FAR20A5, (u16)(r0 + 4)), rb(G_FAR20A5, (u16)(r0 + 5)), x / 32, y / 16);
    }
}
