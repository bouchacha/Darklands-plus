// titleview.cpp - the start screen, drawn by Darklands+ (mods/titleview.c runs it): laid out as
// the game lays out a card, the words on the parchment at the left and a picture at the right
// (the castle at dusk the game shows on the road to a town, pics\aproch1.pic), the Darklands
// lettering of the original start screen (pics\startscr.pic) cut from its stone. The choices:
// Continue (the newest save, named), Load, a new party, Quickstart, Settings, Quit.
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <string>
#include "nsdraw.h"
#include "gamepics.h"
extern "C" {
#include "../runtime/cpu.h"
#include "../mods/titleview.h"
#include "../mods/gamemodel.h"
}


static std::string written(long long t)          /* when a save was written, on this computer */
{
    if (t <= 0) return "";
    time_t tt = (time_t)t, now = time(nullptr);
    struct tm a = *localtime(&tt), b = *localtime(&now);
    char s[64];
    if (a.tm_year == b.tm_year && a.tm_yday == b.tm_yday) snprintf(s, sizeof s, "saved today at %d:%02d", a.tm_hour, a.tm_min);
    else if (a.tm_year == b.tm_year && a.tm_yday + 1 == b.tm_yday) snprintf(s, sizeof s, "saved yesterday at %d:%02d", a.tm_hour, a.tm_min);
    else snprintf(s, sizeof s, "saved %d %s %d", a.tm_mday, gm_month_name(a.tm_mon % 12), a.tm_year + 1900);
    return s;
}

extern "C" void titleview_draw(void)
{
    const TitleModel &m = title_model;
    if (!m.open || ns_screen != NS_TITLE || ns_hidden() || m.busy) return;
    NsDraw P;
    if (!P.begin()) return;
    ImFont *f = P.f, *fo = P.fo;
    const ImU32 ink = NsDraw::INK, soft = NsDraw::INK_SOFT, dim = NsDraw::INK_DIM;
    const ImU32 PARCH = IM_COL32(242, 230, 200, 255);

    /* the parchment, and the picture at the right fading into it */
    P.rect(0, 0, 320, 200, PARCH);
    const float PX0 = 316 - 251, PX1 = 316, PY0 = 4, PY1 = 196;
    if (const GamePic *g = gamepic_part("PICS\\APROCH1.PIC", 68, 7, 242, 185)) {
        SDL_SetTextureScaleMode(g->tex, SDL_ScaleModeLinear);    /* (a watercolour: soft, not in blocks) */
        P.dl->AddImage((ImTextureID)(intptr_t)g->tex, NsDraw::w(PX0, PY0), NsDraw::w(PX1, PY1));
    }
    {
        ImU32 solid = PARCH, clear = PARCH & 0x00ffffffu;
        P.dl->AddRectFilled(NsDraw::w(0, 0), NsDraw::w(PX0 + 50, 200), solid);
        P.dl->AddRectFilledMultiColor(NsDraw::w(PX0 + 50, 0), NsDraw::w(PX0 + 105, 200), solid, clear, clear, solid);
    }
    P.frame(1.5f, 1.5f, 318.5f, 198.5f, IM_COL32(120, 80, 36, 150), 2);

    /* the lettering of the original start screen, and the time and place of the game */
    const float LX0 = 10, LX1 = 142;
    if (const GamePic *g = gamepic_cutout("PICS\\STARTSCR.PIC", 68, 6, 206, 42, 24)) {
        float w = LX1 - LX0, h = w * 42.0f / 206.0f;
        ImTextureID t = (ImTextureID)(intptr_t)g->tex;
        auto at = [&](float dx, float dy, ImU32 tint) { P.dl->AddImage(t, NsDraw::w(LX0 + dx, 9 + dy), NsDraw::w(LX0 + w + dx, 9 + h + dy), ImVec2(0, 0), ImVec2(1, 1), tint); };
        at(1.6f, 1.8f, IM_COL32(60, 34, 20, 110));            /* (on parchment, not stone: a shadow and a dark rim) */
        for (int k = 0; k < 8; k++) at(0.55f * cosf(k * 0.785f), 0.55f * sinf(k * 0.785f), IM_COL32(74, 40, 24, 255));
        at(0, 0, IM_COL32_WHITE);
    }
    const char *tag = "Medieval Germany, the fifteenth century";
    P.text(fo, NsDraw::FS_SMALL, LX0 + (LX1 - LX0 - P.width(fo, NsDraw::FS_SMALL, tag)) * 0.5f, 38, soft, tag);

    /* the choices */
    struct Item { int action; std::string title, sub; bool on; };
    Item items[6]; int n = 0;
    if (m.nsaves > 0) {
        const SaveInfo &s = m.last;
        std::string who;
        for (int i = 0; i < s.nmembers && i < 5; i++) who += (i ? ", " : "") + ns_gtext(s.members[i]);
        char b[200];
        snprintf(b, sizeof b, "%s, %d %s %d", ns_gtext(s.place).c_str(), s.day, gm_month_name((unsigned)s.month % 12), s.year);
        std::string sub = b;
        if (!who.empty()) sub += " - " + who;
        items[n++] = { TV_CONTINUE, "Continue", sub, m.can_continue != 0 };
        snprintf(b, sizeof b, "%d saved game%s to choose from (The Story Continues)", m.nsaves, m.nsaves == 1 ? "" : "s");
        items[n++] = { TV_LOAD, "Load a saved game", b, true };
    } else items[n++] = { TV_LOAD, "Load a saved game", "No saved games yet", false };
    items[n++] = { TV_NEW, "A new party", "Create a New World: up to four characters of your own", true };
    items[n++] = { TV_QUICK, "Quickstart", "Start at once with four ready-made characters", true };
    items[n++] = { TV_SETTINGS, "Settings", "Darklands+ settings, mods, New Player Guide (F12)", true };
    items[n++] = { TV_QUIT, "Quit", "Back to the desktop", true };

    float y = 50, ih = 21;
    for (int i = 0; i < n; i++) {
        const Item &it = items[i];
        bool h = it.on && P.over(LX0 - 3, y, LX1 + 6, y + ih - 2);
        if (h) P.rect(LX0 - 3, y, LX1 + 6, y + ih - 2, IM_COL32(226, 204, 160, 255), 2);
        if ((it.action == TV_CONTINUE && i == 0) || (it.action == TV_NEW && m.nsaves == 0)) P.rect(LX0 - 3, y + 1.5f, LX0 - 1.8f, y + ih - 3.5f, NsDraw::RED);   /* (what Enter does) */
        P.text(f, NsDraw::FS_HEAD + 0.6f, LX0, y + 2.2f, it.on ? ink : dim, it.title);
        P.text(fo, NsDraw::FS_SMALL, LX0, y + 11.2f, it.on ? soft : dim, it.sub, -1, LX1 + 5);
        if (it.action == TV_CONTINUE) {
            std::string t = written(m.last.mtime);
            if (!t.empty()) P.text(fo, NsDraw::FS_SMALL, 0, y + 3.6f, dim, t, LX1 + 4);
        }
        if (it.on && P.hit(LX0 - 3, y, LX1 + 6, y + ih - 2)) ns_push(it.action, 0, 0, 0);
        y += ih;
    }
    P.text(fo, NsDraw::FS_SMALL, LX0, 189, dim, m.can_continue ? "Enter continues;  L load,  N new party,  Q Quickstart" : m.nsaves ? "L load,  N new party,  Q Quickstart" : "Enter or N: a new party;  Q Quickstart");
    {   /* the version, at the foot of the page (over the picture's dark ground, in its light) */
        extern const char dlp_version[], dlp_release[];
        char v[96]; snprintf(v, sizeof v, dlp_release[0] ? "Darklands+ %s (build %s)" : "Darklands+ build %s%s", dlp_release[0] ? dlp_release : dlp_version, dlp_release[0] ? dlp_version : "");
        P.text(fo, NsDraw::FS_SMALL, 0, 189, IM_COL32(226, 210, 170, 200), v, 312);
    }
}
