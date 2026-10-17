/* gamemodel.c - see gamemodel.h */
#include <string.h>
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "gamemodel.h"

int gm_party_size(void) { int n = (s16)g_rw(G_PARTY_SIZE); return n < 0 ? 0 : n > 5 ? 5 : n; }
int gm_leader(void) { return (s8)g_rb(G_LEADER); }
int gm_member_status(int m) { return m >= 0 && m < 5 ? g_rb((u16)(G_MEMBER(m) + G_M_STATUS)) : 0; }
int gm_member_present(int m) { return m >= 0 && m < gm_party_size() && gm_member_status(m) != 0; }
void gm_member_name(int m, char *out, int n) { if (m < 0 || m >= 5) { out[0] = 0; return; } g_str(G_DGROUP, (u16)(G_MEMBER(m) + G_M_SHORTNAME), out, n); }
void gm_member_fullname(int m, char *out, int n) { if (m < 0 || m >= 5) { out[0] = 0; return; } g_str(G_DGROUP, (u16)(G_MEMBER(m) + G_M_FULLNAME), out, n); }
int gm_member_female(int m) { return m >= 0 && m < 5 && g_rb((u16)(G_MEMBER(m) + 3)) != 0; }
void gm_money(int *fl, int *gr, int *pf) { *fl = (s16)g_rw(G_MONEY); *gr = (s16)g_rw(G_MONEY + 2); *pf = (s16)g_rw(G_MONEY + 4); }

int gm_item_count(int m)
{
    if (m < 0 || m >= 5) return 0;
    int n = g_rb((u16)(G_MEMBER(m) + G_M_NITEMS));
    return n > 64 ? 64 : n;
}
int gm_item(int m, int i, GmItem *it)
{
    if (i < 0 || i >= gm_item_count(m)) return 0;
    u16 r = G_ITEM(m, i);
    it->id = rw(G_FAR2E38, r) & 0xfff;
    it->type = rb(G_FAR2E38, (u16)(r + 2));
    it->qual = rb(G_FAR2E38, (u16)(r + 3));
    it->count = rb(G_FAR2E38, (u16)(r + 4));
    it->weight = rb(G_FAR2E38, (u16)(r + 5));
    return 1;
}
/* the member record keeps what is in use as (item type, quality) pairs: +3d/+44, +0e/+46,
 * +37/+3b, +38/+3c, +48/+47 (weapon, armour, ... missile weapon); a record is in use when one
 * of them names its type and quality */
int gm_item_equipped(int m, int i)
{
    GmItem it;
    if (!gm_item(m, i, &it)) return 0;
    u16 b = G_MEMBER(m);
    static const u8 T[5] = { 0x3d, 0x0e, 0x37, 0x38, 0x48 }, Q[5] = { 0x44, 0x46, 0x3b, 0x3c, 0x47 };
    for (int k = 0; k < 5; k++) {
        int t = (s8)g_rb((u16)(b + T[k]));
        if (t >= 0 && t == it.type && g_rb((u16)(b + Q[k])) == it.qual) return 1;
    }
    return 0;
}

/* the shared pack (inventory.shared): each member rides their best mount, a horse or mule (item
   type 108, told by the record itself: in a battle the item definitions are not in memory). It is
   theirs and in use, like what they hold or wear, not one of the party's spares. */
int sheetview_shared(void);
int gm_item_riding(int m, int i)
{
    GmItem g;
    if (!sheetview_shared() || !gm_item(m, i, &g) || g.type != 108) return 0;
    int best = -1, bq = -1;
    for (int j = 0; j < gm_item_count(m); j++) { GmItem x; if (gm_item(m, j, &x) && x.type == 108 && x.qual > bq) { bq = x.qual; best = j; } }
    return best == i;
}
int gm_item_in_use(int m, int i) { return gm_item_equipped(m, i) || gm_item_riding(m, i); }
int gm_item_spare(int m, int i)
{
    GmItem g;
    if (!gm_item(m, i, &g)) return 0;
    return g.count - (gm_item_in_use(m, i) ? 1 : 0);
}
int gm_pack_find(int prefer, int id, int qual, int *m_out, int *i_out)
{
    int bm = -1, bi = -1, bs = 0;
    for (int m = 0; m < 5; m++) {
        if (!gm_member_present(m) || gm_member_status(m) == 5) continue;
        for (int i = 0; i < gm_item_count(m); i++) {
            GmItem g; if (!gm_item(m, i, &g) || g.id != id || g.qual != qual) continue;
            int spare = gm_item_spare(m, i);
            if (spare <= 0) continue;
            if (m == prefer) { *m_out = m; *i_out = i; return 1; }
            if (spare > bs) { bs = spare; bm = m; bi = i; }
        }
    }
    if (bm < 0) return 0;
    *m_out = bm; *i_out = bi;
    return 1;
}

int gm_knows_formula(int m, int f)            /* a byte per potion, a bit per formula of it */
{
    return m >= 0 && m < 5 && f >= 0 && f < 66 && ((rb(G_FAR2E38, (u16)(0x07be + m * 22 + f / 3)) >> (f % 3)) & 1);
}
int gm_knows_saint(int m, int s)              /* 0x14 bytes each, highest bit first */
{
    return m >= 0 && m < 5 && s >= 0 && s < 160 && (rb(G_FAR2E38, (u16)(0x082c + m * 0x14 + (s >> 3))) & (0x80 >> (s & 7)));
}

int gm_room_for(int m, int id, int qual)
{
    int n = gm_item_count(m);
    if (n < 64) return 1;
    u32 d = g_itemdef(id & 0xfff);
    int type = d ? rb((u16)(d >> 16), (u16)((u16)d + 0x1e)) : -1;
    for (int j = 0; j < n; j++) {
        u16 o = G_ITEM(m, j);
        if (rw(G_FAR2E38, o) == (u16)id && rb(G_FAR2E38, (u16)(o + 2)) == type && rb(G_FAR2E38, (u16)(o + 3)) == qual && rb(G_FAR2E38, (u16)(o + 4)) < 0xff) return 1;
    }
    return 0;
}

static u32 itemdef(int id) { return g_itemdef(id & 0xfff); }
void gm_itemdef_name(int id, char *out, int n) { u32 d = itemdef(id); if (!d) { out[0] = 0; return; } g_str((u16)(d >> 16), (u16)d, out, n); }
int gm_itemdef_weight(int id) { u32 d = itemdef(id); return d ? rb((u16)(d >> 16), (u16)((u16)d + 0x25)) : 0; }
int gm_itemdef_no_trade(int id) { u32 d = itemdef(id); return d ? (rb((u16)(d >> 16), (u16)((u16)d + 0x24)) & 0x80) != 0 : 1; }
int gm_itemdef_cats(int id, unsigned long *cats)
{
    u32 d = itemdef(id); if (!d) return 0;
    *cats = (unsigned long)rw((u16)(d >> 16), (u16)((u16)d + 0x20)) | ((unsigned long)rw((u16)(d >> 16), (u16)((u16)d + 0x22)) << 16);
    return 1;
}

void gm_member_attr(int m, int k, int *cur, int *max)
{
    *cur = *max = 0;
    if (m < 0 || m >= 5 || k < 0 || k > 6) return;
    *cur = (s8)g_rb((u16)(G_MEMBER(m) + G_M_ATTR_CUR + k));
    *max = (s8)g_rb((u16)(G_MEMBER(m) + G_M_ATTR_MAX + k));
}
int gm_member_skill(int m, int k) { return m >= 0 && m < 5 && k >= 0 && k < 19 ? (s8)g_rb((u16)(G_MEMBER(m) + G_M_SKILLS + k)) : 0; }
int gm_member_age(int m) { return m >= 0 && m < 5 ? (s16)g_rw((u16)(G_MEMBER(m) - 2)) : 0; }   /* (the int before the record) */
int gm_member_load(int m) { return m >= 0 && m < 5 ? g_rb((u16)(G_MEMBER(m) + G_M_LOAD)) : 0; }
/* the sheet's load: 0 light (up to half of END + STR), 1 normal (up to all), 2 heavy (half again), 3 overloaded
 * (s029 0e76:013a) */
int gm_member_load_band(int m)
{
    int cur, mx, e, s;
    gm_member_attr(m, 0, &e, &mx); gm_member_attr(m, 1, &s, &mx);
    unsigned max = (unsigned)(e + s); cur = gm_member_load(m);
    if (max / 2 >= (unsigned)cur) return 0;
    if ((unsigned)cur <= max) return 1;
    if ((unsigned)cur <= max * 3 / 2) return 2;
    return 3;
}
char gm_member_herald_letter(int m)
{
    if (m < 0 || m >= 5) return 0;
    int c = g_rb((u16)(G_MEMBER(m) + 1));
    if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
    return c >= 'a' && c <= 'z' ? (char)c : 0;
}
void gm_member_figure_code(int m, char *out, int n)
{
    out[0] = 0;
    if (m < 0 || m >= 5 || n < 4) return;
    for (int i = 0; i < 3; i++) { int c = g_rb((u16)(GV_member_shield + 4 * m + i)); if (c < 33 || c > 126) { out[0] = 0; return; } out[i] = (char)c; }
    out[3] = 0;
}
void gm_member_palette(int m, unsigned char out[24])
{
    for (int i = 0; i < 24; i++) out[i] = (m < 0 || m >= 5) ? 0 : (unsigned char)(rb(G_FAR20A5, (u16)(0x33fc + 24 * m + i)) & 63);   /* member_palette, far 20a5:33fc */
}
void gm_member_equip(int m, GmEquip *e)
{
    u16 b = G_MEMBER(m < 0 || m >= 5 ? 0 : m);
    e->weapon = (s8)g_rb((u16)(b + 0x3d)); e->vital = (s8)g_rb((u16)(b + 0x37)); e->limb = (s8)g_rb((u16)(b + 0x38));
    e->shield = (s8)g_rb((u16)(b + 0x48)); e->missile = (s8)g_rb((u16)(b + 0x0e));
}
int gm_in_battle(void) { return g_rb(GV_in_battle) == 1; }
int gm_member_elsewhere(int m) { return gm_in_battle() && m >= 0 && m < 5 && (u8)g_rb((u16)(GV_cbt_floor + m)) == 0x9d; }
int gm_member_selected(int m) { return m >= 0 && m < 5 && g_rb((u16)(GV_cbt_selected + m)) != 0; }
/* the letter the game's panel shows (root member_status_letter): the order being given, or the
 * one in force; 0x400: an action at a place (stairs, door ...) */
char gm_member_order(int m)
{
    if (!gm_in_battle() || m < 0 || m >= 5 || gm_member_status(m) == 5) return 0;
    { int s2 = g_rb((u16)(GV_cbt_state + m)); if (s2) return s2 == 1 ? 'b' : s2 == 2 ? 'e' : 0; }
    int st;
    int paused = g_rb(GV_ai_paused) == 1, giving = g_rb(GV_order_mode) == 1 && (s8)g_rb(GV_selected_member) == m;
    if (paused && giving && (s16)g_rw(GV_order_action) != -1) st = (s16)g_rw(GV_order_action);
    else st = (s16)g_rw((u16)(GV_combatants + m * 0x80 + 0x10));
    if (st == 0x400) {
        int sp = (paused && giving && (s16)g_rw(GV_order_special) != -1) ? (s16)g_rw(GV_order_special) : (s8)g_rb((u16)(GV_cbt_special + m));
        switch (sp) { case 1: case 2: return 'U'; case 3: case 6: return 'P'; case 4: case 7: return 'D'; case 5: return 'O'; case 8: return 'X'; case 10: return 'H'; }
        return 0;
    }
    switch (st) {
    case 0: return '-';
    case 1: return (g_rw((u16)(GV_cbt_flags + 2 * m)) & 0x1000) ? 'F' : 'W';
    case 6: return 'B'; case 0x0a: return 'V'; case 0x12: return 'A'; case 0x22: return 'P';
    case 0x40: return 'R'; case 0x80: return 'M'; case 0x100: return 'T';
    }
    return 0;
}
