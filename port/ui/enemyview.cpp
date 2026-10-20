// enemyview.cpp - the battle's Enemy Info drawn by Darklands+ (mods/enemyview.c runs it): every
// kind of foe on the field with its picture, how many and how many still fight, its armour,
// shield and weapons (with the game's word for its skill, but not at Expert), and what each of
// the party's weapons does against its body armour.
#include <stdio.h>
#include <string>
#include "nsdraw.h"
#include "gamepics.h"
extern "C" {
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../mods/enemyview.h"
#include "../mods/gamemodel.h"
}

static std::string type_name(int t, int k1, int k2 = -1)
{
    if (t < 0) return "";
    for (const ItemDef &d : gamedata.items)
        if (d.type == t && ((int)d.kind == k1 || (int)d.kind == k2)) {
            return game_text(d.plain.c_str(), d.plain.size());
        }
    return "";
}

extern "C" void enemyview_draw(void)
{
    const EnemyModel &m = enemy_model;
    if (!m.open || ns_screen != NS_ENEMY || ns_hidden()) return;
    NsDraw P;
    if (!P.begin()) return;
    ImFont *f = P.f, *fo = P.fo;
    const ImU32 ink = NsDraw::INK, soft = NsDraw::INK_SOFT, dim = NsDraw::INK_DIM;
    static const char *HN[6] = { "no armour", "soft", "stiff", "scale", "mail", "plate" };
    int cols = m.n > 3 ? 2 : 1, rows = (m.n + cols - 1) / cols;
    const float X0 = 64, X1 = 318, Y0 = 3;
    float ch = (197 - Y0 - 16 - (rows - 1) * 3) / (rows ? rows : 1);
    if (ch > 62) ch = 62;
    const float Y1 = Y0 + 14 + rows * (ch + 3) + 1;
    P.rect(60, 0, 320, 200, IM_COL32(20, 12, 6, 120));
    P.rect(X0, Y0, X1, Y1, IM_COL32(242, 230, 200, 252), 2);
    P.frame(X0, Y0, X1, Y1, IM_COL32(120, 80, 36, 180), 2);
    P.text(f, NsDraw::FS_TITLE, X0 + 4, Y0 + 1.5f, ink, "The enemy");
    P.text(fo, NsDraw::FS_SMALL, 0, Y0 + 4, dim, "press a key or click to return to the battle", X1 - 4);
    float cw = (X1 - X0 - 6 - (cols - 1) * 3) / cols;
    int size = gm_party_size(); if (size < 0 || size > 5) size = 0;
    for (int i = 0; i < m.n; i++) {
        const EnemyKind &e = m.k[i];
        float x = X0 + 3 + (i % cols) * (cw + 3), y = Y0 + 14 + (i / cols) * (ch + 3);
        P.rect(x, y, x + cw, y + ch, IM_COL32(234, 220, 190, 255), 1.5f);
        float tx = x + 3;
        if (const GamePic *g = gamepic(e.pic)) {
            float ph = fminf(ch - 6, 34.0f), pw = g->w * ph / (g->h ? g->h : 1);
            P.dl->AddImage((ImTextureID)(intptr_t)g->tex, NsDraw::w(x + 3, y + 3), NsDraw::w(x + 3 + pw, y + 3 + ph));
            tx = x + 6 + pw;
        }
        P.text(f, NsDraw::FS_HEAD, tx, y + 1.5f, ink, ns_gtext(e.name), -1, x + cw - 2);
        char b[160];
        if (e.count > 1) snprintf(b, sizeof b, "%d on the field, %d still fighting", e.count, e.able);
        else snprintf(b, sizeof b, "%s", e.able ? "still fighting" : "down");
        P.text(fo, NsDraw::FS_SMALL, tx, y + 9.5f, soft, b);
        float ly = y + 15.5f, lw = x + cw - 3 - tx;
        auto line = [&](const std::string &s, ImU32 c) { if (ly < y + ch - 4) ly += P.para(fo, 4.4f, tx, ly, lw, c, s) + 0.3f; };
        std::string word_m = m.difficulty != 2 ? std::string(" - ") + m.quality[e.melee_skill < 7 ? e.melee_skill : 0] : "";
        std::string word_s = m.difficulty != 2 ? std::string(" - ") + m.quality[e.missile_skill < 7 ? e.missile_skill : 0] : "";
        int h = 0;
        if (e.human) {
            std::string a;
            if (e.vital >= 67 && e.vital <= 84) { h = rb(G_FAR20A5, (u16)(0x781e + e.vital)); a = "Body: " + type_name(e.vital, K_ARMOR) + " (" + (h >= 0 && h < 6 ? HN[h] : "?") + ")"; }
            else a = "Body: none";
            if (e.limb >= 67 && e.limb <= 84) a += ", legs: " + type_name(e.limb, K_ARMOR);
            if (e.shield >= 0) a += ", shield: " + type_name(e.shield, K_SHIELD);
            if (e.armour_magic) a += " (enchanted)";
            line(a, ink);
            std::string w;
            if (e.melee >= 0 && e.melee <= 62) w = type_name(e.melee, K_WEAPON, K_MISSILE) + word_m;
            if (e.missile >= 0 && e.missile <= 62) w += (w.empty() ? "" : "; ") + type_name(e.missile, K_WEAPON, K_MISSILE) + word_s;
            if (e.weapon_magic && !w.empty()) w += " (enchanted)";
            if (!w.empty()) line(w, ink);
        } else {
            line(std::string("Natural armour") + (m.difficulty != 2 ? std::string(" - ") + m.quality[e.natural] : ""), ink);
            line("Natural weapons" + word_m, ink);
        }
        /* the party's weapons against its body armour (the game's rule: penetration above the
           hardness pierces, equal is halved, below is deflected) */
        if (e.human) {
            for (int p = 0; p < size; p++) {
                if (!gm_member_present(p) || gm_member_status(p) != 1) continue;
                std::string s; int best = 3;
                char nm[24]; gm_member_name(p, nm, sizeof nm);
                for (int k = 0; k < 2; k++) {
                    int mw = (s8)g_rb((u16)(G_MEMBER(p) + (k ? 0x0e : 0x3d)));
                    if (mw < 0 || mw > 62) continue;
                    int pen = rb(G_FAR20A5, (u16)(0x76a8 + mw)) & 15, r = pen > h ? 0 : pen == h ? 1 : 2;
                    if (r < best) best = r;
                    s += std::string(s.empty() ? "" : "; ") + type_name(mw, K_WEAPON, K_MISSILE) + (r == 0 ? " pierces" : r == 1 ? " does half damage" : " is deflected");
                }
                if (best == 3) continue;
                ImU32 c = best == 0 ? IM_COL32(40, 96, 34, 255) : best == 1 ? IM_COL32(150, 100, 10, 255) : NsDraw::RED;
                line(ns_gtext(nm) + ": " + s, c);
            }
        }
    }
}
