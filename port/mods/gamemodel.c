/* gamemodel.c - see gamemodel.h */
#include <string.h>
#include "../runtime/cpu.h"
#include "../runtime/game.h"
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
