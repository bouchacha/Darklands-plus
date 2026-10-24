// savesview.cpp - the game's Save and Load drawn by Darklands+ (mods/savesview.c runs them): the
// comment to save under, and the saves there are, newest first, each with its comment, where
// and when in the game, who, the purse and how long ago it was written. Save: a click on a save
// makes the new one take its place; Load: the chosen save has a Delete (twice).
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <math.h>
#include <string>
#include "nsdraw.h"
#include "drawcache.h"
#include "alchemydesk.h"
#include "money.h"
extern "C" {
#include "../runtime/cpu.h"
#include "../mods/savesview.h"
#include "../mods/titleview.h"
#include "../mods/gamemodel.h"
u32 plat_ticks_ms(void);
}


static std::string ago(long long t)
{
    if (t <= 0) return "";
    long long d = (long long)time(nullptr) - t;
    char b[48];
    if (d < 60) snprintf(b, sizeof b, "just now");
    else if (d < 3600) snprintf(b, sizeof b, "%lld min ago", d / 60);
    else if (d < 86400) snprintf(b, sizeof b, "%lld h ago", d / 3600);
    else if (d < 86400 * 2) snprintf(b, sizeof b, "yesterday");
    else if (d < 86400 * 60) snprintf(b, sizeof b, "%lld days ago", d / 86400);
    else { struct tm *tm = localtime((time_t *)&t); strftime(b, sizeof b, "%Y-%m-%d", tm); }
    return b;
}

extern "C" int savesview_showing(int *loading)
{
    if (!saves_model.open || ns_screen != NS_SAVES || ns_hidden()) return 0;
    if (loading) *loading = saves_model.loading;
    return 1;
}

extern "C" void saves_prefs(void);   /* ui/saves.cpp */
extern "C" void savesview_draw(void)
{
    SavesModel &S = saves_model;
    if (!savesview_showing(nullptr)) return;
    NsDraw P;
    if (!P.begin()) return;
    ImFont *f = P.f, *fo = P.fo;
    const ImU32 ink = NsDraw::INK, soft = NsDraw::INK_SOFT, dim = NsDraw::INK_DIM;
    /* at the start screen (no party, no panel at the left) the page is the whole screen */
    bool full = title_model.open != 0;
    if (full) {
        P.rect(0, 0, 320, 200, IM_COL32(236, 222, 188, 255));
        P.rect(1.5f, 1.5f, 318.5f, 198.5f, IM_COL32(242, 230, 200, 255), 2);
        P.frame(1.5f, 1.5f, 318.5f, 198.5f, IM_COL32(120, 80, 36, 150), 2);
    } else {                                                     /* the register on the table (ui/alchemydesk.cpp's oak) */
        ImVec2 o = NsDraw::w(60, 0), e = NsDraw::w(320, 200);
        dc_draw(P.dl, o, e, 0x5a7e5ull, [&](ImDrawList *dl) { desk_table(dl); });
    }
    const ImU32 ink_t = full ? ink : IM_COL32(246, 232, 200, 255), soft_t = full ? soft : IM_COL32(226, 210, 180, 235);   /* (on the table: light letters) */
    const float L = full ? -60 : 0;                              /* (the left edge: 60 further left) */
    static unsigned seq; static float scroll; static int last_click_row = -1; static u32 last_click_t;
    if (seq != S.seq) { seq = S.seq; scroll = 0; last_click_row = -1; }

    P.text(f, NsDraw::FS_TITLE, 65 + L, 3, ink_t, S.loading ? "Load a saved game" : "Save the game");
    float bx = 284;
    if (P.button(bx, 4, 315, 15, "Cancel", IM_COL32(112, 40, 22, 235), NsDraw::FS_HEADBTN, PB_Y)) savesview_push(SVV_CANCEL, 0);
    if (S.loading) { if (S.sel >= 0 && P.button(bx - 34, 4, bx - 3, 15, "Load", IM_COL32(40, 92, 34, 235), NsDraw::FS_HEADBTN, PB_X)) savesview_push(SVV_OK, S.sel); }
    else if (P.button(bx - 34, 4, bx - 3, 15, "Save", IM_COL32(40, 92, 34, 235), NsDraw::FS_HEADBTN, PB_X)) savesview_push(SVV_OK, 0);
    static bool opts;                                         /* the saving options (quick save, autosaves): here, by the saves */
    if (P.button(bx - 94, 4, bx - 38, 15, opts ? "Close options" : "Saving options", IM_COL32(120, 92, 70, 220), NsDraw::FS_HEADBTN)) opts = !opts;
    if (opts) {
        ImGuiIO &io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(fminf(io.DisplaySize.x * 0.8f, 900), 0), ImGuiCond_Appearing);
        if (ImGui::Begin("Saving options", &opts, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize)) saves_prefs();
        ImGui::End();
    }

    float LY0 = 22;
    if (!S.loading) {
        P.text(fo, NsDraw::FS_BODY, 65 + L, 15.5f, soft_t, "Save name (up to 22 characters):");
        P.rect(64 + L, 23, 316, 33, IM_COL32(255, 250, 236, 255), 1.5f);
        P.frame(64 + L, 23, 316, 33, IM_COL32(120, 80, 36, 160), 1.5f);
        std::string c = S.comment;
        if (S.fresh && !c.empty()) { float w = P.width(f, NsDraw::FS_NAME, c); P.rect(66.2f + L, 24.2f, 66.8f + L + w, 31.8f, IM_COL32(226, 200, 150, 255), 0.6f); }
        P.text(f, NsDraw::FS_NAME, 66.5f + L, 24.4f, ink, c);
        if (((plat_ticks_ms() - S.caret_t) / 500) % 2 == 0) {
            float cx = 66.8f + L + P.width(f, NsDraw::FS_NAME, c);
            P.rect(cx, 24.5f, cx + 0.6f, 31.5f, ink);
        }
        LY0 = 40;
        P.text(f, NsDraw::FS_HEAD, 65 + L, LY0 - 3.5f, ink_t, "Existing saves");
        P.text(fo, NsDraw::FS_SMALL, 0, LY0 - 2.2f, soft_t, S.sel >= 0 ? "click it again not to replace it" : "click one to replace it with this save", 316);
        LY0 += 5;
    }
    else P.text(fo, NsDraw::FS_SMALL, 65 + L, 15.5f, soft_t, "Double-click a save to load it.");

    const float X0 = 63 + L, X1 = 317, Y1 = 196, ROWH = 15.5f;
    if (!full) {                                                 /* the register: bound in leather, stitched at the spine, its ribbon */
        P.dl->AddRectFilled(NsDraw::w(X0 - 1, LY0 - 1.5f), NsDraw::w(X1 + 2.5f, Y1 + 3.5f), IM_COL32(0, 0, 0, 100), 2.5f * P.k);
        P.dl->AddRectFilledMultiColor(NsDraw::w(X0 - 2.5f, LY0 - 2.2f), NsDraw::w(X1 + 1.5f, Y1 + 2.5f), IM_COL32(100, 52, 28, 255), IM_COL32(90, 46, 24, 255), IM_COL32(60, 28, 14, 255), IM_COL32(70, 34, 18, 255));
        P.dl->AddRect(NsDraw::w(X0 - 1.6f, LY0 - 1.6f), NsDraw::w(X1 + 0.6f, Y1 + 1.6f), IM_COL32(214, 170, 80, 140), 1.5f * P.k, 0, 0.8f);
    }
    P.rect(X0, LY0 - 1, X1, Y1 + 1, IM_COL32(250, 242, 222, 255), 1.5f);
    P.frame(X0, LY0 - 1, X1, Y1 + 1, IM_COL32(120, 80, 36, 90), 1.5f);
    if (!full) {
        for (float yy = LY0 + 2; yy < Y1; yy += 6) P.dl->AddLine(NsDraw::w(X0 - 2, yy), NsDraw::w(X0 + 1.2f, yy + 1.5f), IM_COL32(200, 180, 140, 220), fmaxf(1.0f, 0.5f * P.k));   /* (the stitches) */
    }
    int vis = (int)((Y1 - LY0) / ROWH);
    float maxs = S.nsaves > vis ? (float)(S.nsaves - vis) : 0;
    if (P.over(X0, LY0, X1, Y1) && ns_in.wheel != 0) { scroll -= ns_in.wheel; ns_in.wheel = 0; }
    if (S.loading && S.sel >= 0) {                            /* the chosen one in sight (keys) */
        static int last_sel = -2;
        if (S.sel != last_sel) { if (S.sel < scroll) scroll = (float)S.sel; if (S.sel >= scroll + vis) scroll = (float)(S.sel - vis + 1); last_sel = S.sel; }
    }
    scroll = fmaxf(0, fminf(scroll, maxs));
    if (!S.nsaves) P.text(fo, NsDraw::FS_BODY, X0 + 3, LY0 + 3, dim, "No saved games yet.");
    P.dl->PushClipRect(NsDraw::w(X0, LY0), NsDraw::w(X1, Y1), true);
    int first = (int)scroll;
    for (int i = first; i < S.nsaves && i < first + vis + 1; i++) {
        const SaveInfo &v = S.saves[i];
        float y = LY0 + (i - first) * ROWH;
        bool sel = i == S.sel, h = P.over(X0, y, X1, y + ROWH) && P.over(X0, LY0, X1, Y1);
        bool cur = !strcasecmp(v.file, S.cur_file);
        if (sel && !S.loading) P.rect(X0 + 0.5f, y, X1 - 0.5f, y + ROWH, IM_COL32(232, 196, 170, 255), 1.0f);     /* (to be replaced) */
        else if (sel) P.rect(X0 + 0.5f, y, X1 - 0.5f, y + ROWH, IM_COL32(226, 200, 150, 255), 1.0f);
        else if (h) P.rect(X0 + 0.5f, y, X1 - 0.5f, y + ROWH, IM_COL32(240, 226, 196, 255), 1.0f);
        else if (i % 2) P.rect(X0 + 0.5f, y, X1 - 0.5f, y + ROWH, IM_COL32(244, 234, 210, 255));
        std::string title = ns_gtext(v.comment);
        if (title.empty()) title = ns_gtext(v.place);
        float tx = X0 + 3;
        P.text(f, NsDraw::FS_NAME, tx, y + 1.0f, ink, title, -1, X1 - 72);
        char when[64]; snprintf(when, sizeof when, "%d %s %d, %d:00", v.day, gm_month_name((unsigned)v.month % 12), v.year, v.hour);
        P.text(fo, NsDraw::FS_SMALL, 0, y + 1.8f, soft, when, X1 - 3);
        bool armed = S.loading && S.armed == i && plat_ticks_ms() - S.armed_t <= 4000, del_hit = false;
        if (S.loading && sel && !cur) {                           /* Load: the chosen save can be deleted (a second click does it) */
            float wx1 = X1 - 6 - P.width(fo, NsDraw::FS_SMALL, when), wx0 = wx1 - (armed ? 34 : 22);
            del_hit = P.over(wx0, y + 1.2f, wx1, y + 7.6f);
            if (P.button(wx0, y + 1.2f, wx1, y + 7.6f, armed ? "Delete it?" : "Delete", armed ? IM_COL32(150, 40, 22, 240) : IM_COL32(120, 92, 70, 220))) savesview_push(SVV_DELETE, i);
        }
        std::string who;
        for (int m = 0; m < v.nmembers; m++) { if (m) who += ", "; who += ns_gtext(v.members[m]); }
        std::string line = ns_gtext(v.place);
        if (!who.empty()) line += (line.empty() ? "" : "  -  ") + who;
        line += "  -  " + money_fgp(v.florins, v.groschen, v.pfennigs);
        P.text(fo, NsDraw::FS_SMALL, tx, y + 8.6f, dim, line, -1, X1 - 60);
        std::string tag = ago(v.mtime);
        if (v.battle) tag = "in a battle, " + tag;
        if (cur) tag = (S.loading ? "the last loaded or saved, " : "the last used, ") + tag;
        ImU32 tc = cur ? NsDraw::GREEN : dim;
        if (sel && !S.loading) { tag = "will be replaced by this save"; tc = IM_COL32(150, 40, 22, 255); }
        if (armed) { tag = "click again to delete it for good"; tc = IM_COL32(150, 40, 22, 255); }
        P.text(fo, NsDraw::FS_SMALL, 0, y + 8.6f, tc, tag, X1 - 3);
        if (!del_hit && P.hit(X0, y, X1, y + ROWH) && P.over(X0, LY0, X1, Y1)) {
            u32 now = plat_ticks_ms();
            if (S.loading && last_click_row == i && now - last_click_t < 450) savesview_push(SVV_OK, i);
            else savesview_push(SVV_SELECT, i);
            last_click_row = i; last_click_t = now;
        }
    }
    P.dl->PopClipRect();
    if (maxs > 0) {                                           /* where in the list */
        float t = (Y1 - LY0) * vis / S.nsaves, y = LY0 + (Y1 - LY0 - t) * scroll / maxs;
        P.rect(X1 - 2.2f, y, X1 - 0.8f, y + t, IM_COL32(120, 80, 36, 120), 0.7f);
    }
}
