// savesview.cpp - the game's Save and Load drawn by Darklands+ (mods/savesview.c runs them): the
// comment to save under, and the saves there are, newest first, each with its comment, where
// and when in the game, who, the purse and how long ago it was written.
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <math.h>
#include <string>
#include "nsdraw.h"
extern "C" {
#include "../runtime/cpu.h"
#include "../mods/savesview.h"
u32 plat_ticks_ms(void);
}

static const char *MON[12] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };

static std::string ago(long long t)
{
    if (t <= 0) return "";
    long long d = (long long)time(nullptr) - t;
    char b[48];
    if (d < 60) snprintf(b, sizeof b, "just now");
    else if (d < 3600) snprintf(b, sizeof b, "%lld min ago", d / 60);
    else if (d < 86400) snprintf(b, sizeof b, "%lld h ago", d / 3600);
    else if (d < 86400 * 60) snprintf(b, sizeof b, "%lld days ago", d / 86400);
    else { struct tm *tm = localtime((time_t *)&t); strftime(b, sizeof b, "%Y-%m-%d", tm); }
    return b;
}

extern "C" void savesview_draw(void)
{
    SavesModel &S = saves_model;
    if (!S.open || ns_screen != NS_SAVES || ns_hidden()) return;
    NsDraw P;
    if (!P.begin()) return;
    ImFont *f = P.f, *fo = P.fo;
    const ImU32 ink = NsDraw::INK, soft = NsDraw::INK_SOFT, dim = NsDraw::INK_DIM;
    P.page();
    static unsigned seq; static float scroll; static int last_click_row = -1; static u32 last_click_t;
    if (seq != S.seq) { seq = S.seq; scroll = 0; last_click_row = -1; }

    P.text(f, NsDraw::FS_TITLE, 65, 3, ink, S.loading ? "Load a saved game" : "Save the game");
    float bx = 284;
    if (P.button(bx, 4, 315, 15, "Cancel", IM_COL32(112, 40, 22, 235))) savesview_push(SVV_CANCEL, 0);
    if (S.loading) { if (S.sel >= 0 && P.button(bx - 34, 4, bx - 3, 15, "Load", IM_COL32(40, 92, 34, 235))) savesview_push(SVV_OK, S.sel); }
    else if (P.button(bx - 34, 4, bx - 3, 15, "Save", IM_COL32(40, 92, 34, 235))) savesview_push(SVV_OK, 0);

    float LY0 = 22;
    if (!S.loading) {
        P.text(fo, NsDraw::FS_BODY, 65, 15.5f, soft, "A name for it, 22 letters at most (type; Enter saves, Esc goes back):");
        P.rect(64, 23, 316, 33, IM_COL32(255, 250, 236, 255), 1.5f);
        P.frame(64, 23, 316, 33, IM_COL32(120, 80, 36, 160), 1.5f);
        std::string c = S.comment;
        if (S.fresh && !c.empty()) { float w = P.width(f, NsDraw::FS_NAME, c); P.rect(66.2f, 24.2f, 66.8f + w, 31.8f, IM_COL32(226, 200, 150, 255), 0.6f); }
        P.text(f, NsDraw::FS_NAME, 66.5f, 24.4f, ink, c);
        if (((plat_ticks_ms() - S.caret_t) / 500) % 2 == 0) {
            float cx = 66.8f + P.width(f, NsDraw::FS_NAME, c);
            P.rect(cx, 24.5f, cx + 0.6f, 31.5f, ink);
        }
        LY0 = 40;
        P.text(f, NsDraw::FS_HEAD, 65, LY0 - 3.5f, ink, "The saves there are");
        LY0 += 5;
    }
    else P.text(fo, NsDraw::FS_SMALL, 65, 15.5f, soft, "Click a save, then Load (or double-click it); Up / Down and Enter work too. Cancel leaves the game as it is.");

    const float X0 = 63, X1 = 317, Y1 = 196, ROWH = 15.5f;
    P.rect(X0, LY0 - 1, X1, Y1 + 1, IM_COL32(250, 242, 222, 255), 1.5f);
    P.frame(X0, LY0 - 1, X1, Y1 + 1, IM_COL32(120, 80, 36, 90), 1.5f);
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
        bool sel = S.loading && i == S.sel, h = S.loading && P.over(X0, y, X1, y + ROWH) && P.over(X0, LY0, X1, Y1);
        if (sel) P.rect(X0 + 0.5f, y, X1 - 0.5f, y + ROWH, IM_COL32(226, 200, 150, 255), 1.0f);
        else if (h) P.rect(X0 + 0.5f, y, X1 - 0.5f, y + ROWH, IM_COL32(240, 226, 196, 255), 1.0f);
        else if (i % 2) P.rect(X0 + 0.5f, y, X1 - 0.5f, y + ROWH, IM_COL32(244, 234, 210, 255));
        bool cur = !strcasecmp(v.file, S.cur_file);
        std::string title = ns_gtext(v.comment);
        if (title.empty()) title = ns_gtext(v.place);
        float tx = X0 + 3;
        P.text(f, NsDraw::FS_NAME, tx, y + 1.0f, ink, title, -1, X1 - 72);
        char when[64]; snprintf(when, sizeof when, "%d %s %d, %d:00", v.day, MON[(unsigned)v.month % 12], v.year, v.hour);
        P.text(fo, NsDraw::FS_SMALL, 0, y + 1.8f, soft, when, X1 - 3);
        std::string who;
        for (int m = 0; m < v.nmembers; m++) { if (m) who += ", "; who += ns_gtext(v.members[m]); }
        std::string line = ns_gtext(v.place);
        if (!who.empty()) line += (line.empty() ? "" : "  -  ") + who;
        line += "  -  " + std::to_string(v.florins) + " fl";
        P.text(fo, NsDraw::FS_SMALL, tx, y + 8.6f, dim, line, -1, X1 - 60);
        std::string tag = ago(v.mtime);
        if (v.battle) tag = "in a battle, " + tag;
        if (cur) tag = (S.loading ? "the last loaded or saved, " : "the last, ") + tag;
        P.text(fo, NsDraw::FS_SMALL, 0, y + 8.6f, cur ? NsDraw::GREEN : dim, tag, X1 - 3);
        if (S.loading && P.hit(X0, y, X1, y + ROWH) && P.over(X0, LY0, X1, Y1)) {
            u32 now = plat_ticks_ms();
            if (last_click_row == i && now - last_click_t < 450) savesview_push(SVV_OK, i);
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
